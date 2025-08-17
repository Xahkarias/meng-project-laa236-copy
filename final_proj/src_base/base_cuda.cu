#include <algorithm>
#include <iostream>
#include <vector>
#include <cuda_runtime.h>
#include "../defines.hpp"

// This kernel fills the dp matrix based on 3 of its neighbors and penalties
__global__ void smith_waterman_kernel_optimized(
    const char *__restrict__ seq1,
    const char *__restrict__ seq2,
    int *__restrict__ score,
    int size1,
    int size2,
    int diag)
{
    // shared memory used by threads in a block
    extern __shared__ int shared_score[];
    // current thread parameters
    int thread_id = threadIdx.x;
    int global_idx = blockIdx.x * blockDim.x + threadIdx.x;

    unsigned long long i = diag - global_idx;
    unsigned long long j = global_idx + 1;

    // checking bounds
    if (i < 1 || i > size1 || j < 1 || j > size2) return;

    // Compute indices
    unsigned long long index = i * (size2 + 1) + j;
    unsigned long long index_diag = (i - 1) * (size2 + 1) + (j - 1);
    unsigned long long index_up = (i - 1) * (size2 + 1) + j;
    unsigned long long index_left = i * (size2 + 1) + (j - 1);

    // checking miss MATCH_SCORE or MATCH_SCORE
    char a = seq1[i - 1];
    char b = seq2[j - 1];
    int matchScore = (a == b) ? MATCH_SCORE : MISMATCH_SCORE;

    // Use shared memory to store neighboring values
    shared_score[thread_id] = score[index_diag];

    __syncthreads();

    // computing all the possibilites, removed conditional statements for speed up
    int score_diag = shared_score[thread_id] + matchScore;
    int score_up = score[index_up] + GAP_SCORE;
    int score_left = score[index_left] + GAP_SCORE;

    //updating the matrix
    int cellScore = max(0, max(score_diag, max(score_up, score_left)));
    score[index] = cellScore;
}

// Find the maximum score and its position in the score matrix
__global__ void find_max_score_kernel(
    int *__restrict__ score,
    int *__restrict__ max_i, 
    int *__restrict__ max_j,
    int *__restrict__ max_score,
    int size1,
    int size2)
{
    // Shared memory for reduction
    extern __shared__ int shared_data[];
    int *shared_scores = shared_data;
    int *shared_i = &shared_data[blockDim.x];
    int *shared_j = &shared_data[2 * blockDim.x];
    
    int tid = threadIdx.x;
    int global_idx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;
    
    // Thread-local max
    int local_max_score = 0;
    int local_max_i = 0;
    int local_max_j = 0;
    
    // Process multiple elements per thread
    for (int idx = global_idx; idx < (size1 + 1) * (size2 + 1); idx += stride) {
        int i = idx / (size2 + 1);
        int j = idx % (size2 + 1);
        int current_score = score[idx];
        
        if (current_score > local_max_score) {
            local_max_score = current_score;
            local_max_i = i;
            local_max_j = j;
        }
    }
    
    // Load thread-local max into shared memory
    shared_scores[tid] = local_max_score;
    shared_i[tid] = local_max_i;
    shared_j[tid] = local_max_j;
    
    __syncthreads();
    
    // Reduction in shared memory
    for (int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            if (shared_scores[tid] < shared_scores[tid + s]) {
                shared_scores[tid] = shared_scores[tid + s];
                shared_i[tid] = shared_i[tid + s];
                shared_j[tid] = shared_j[tid + s];
            }
        }
        __syncthreads();
    }
    
    // Write block's result to global memory
    if (tid == 0) {
        int block_max_score = shared_scores[0];
        atomicMax(max_score, block_max_score);
        
        // Use atomicCAS to update max_i and max_j only if this block has the max score
        int old_max = atomicCAS(max_score, block_max_score, block_max_score);
        if (old_max == block_max_score) {
            atomicExch(max_i, shared_i[0]);
            atomicExch(max_j, shared_j[0]);
        }
    }
}

// Traceback kernel to generate alignment paths
__global__ void traceback_kernel(
    const char *__restrict__ seq1,
    const char *__restrict__ seq2,
    int *__restrict__ score,
    int max_i,
    int max_j,
    int size1,
    int size2,
    char *__restrict__ aligned_seq1,
    char *__restrict__ aligned_seq2,
    int *__restrict__ align_length)
{
    // Only one thread does the traceback
    if (threadIdx.x == 0 && blockIdx.x == 0) {
        int i = max_i;
        int j = max_j;
        int idx = 0;
        
        // Temporary arrays to store alignment (will be reversed later)
        char temp_seq1[10000]; // Assuming max length, adjust as needed
        char temp_seq2[10000];
        
        while (i > 0 && j > 0 && score[i * (size2 + 1) + j] > 0) {
            int current_idx = i * (size2 + 1) + j;
            int diag_idx = (i - 1) * (size2 + 1) + (j - 1);
            int up_idx = (i - 1) * (size2 + 1) + j;
            //int left_idx = i * (size2 + 1) + (j - 1);
            
            char a = seq1[i - 1];
            char b = seq2[j - 1];
            int match_score = (a == b) ? MATCH_SCORE : MISMATCH_SCORE;
            
            if (score[current_idx] == score[diag_idx] + match_score) {
                // Diagonal
                temp_seq1[idx] = seq1[i - 1];
                temp_seq2[idx] = seq2[j - 1];
                i--; j--;
            } else if (score[current_idx] == score[up_idx] + GAP_SCORE) {
                // Up
                temp_seq1[idx] = seq1[i - 1];
                temp_seq2[idx] = '-';
                i--;
            } else {
                // Left
                temp_seq1[idx] = '-';
                temp_seq2[idx] = seq2[j - 1];
                j--;
            }
            idx++;
        }
        
        // Reverse the alignment
        for (int k = 0; k < idx; k++) {
            aligned_seq1[k] = temp_seq1[idx - k - 1];
            aligned_seq2[k] = temp_seq2[idx - k - 1];
        }
        
        // Set null terminators
        aligned_seq1[idx] = '\0';
        aligned_seq2[idx] = '\0';
        
        // Set alignment length
        *align_length = idx;
    }
}

std::pair<std::string, std::string> smithWaterman(
    const char *seq1,
    size_t size1,
    const char *seq2,
    size_t size2)
{
    int device;
    cudaGetDevice(&device);

    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, device);
    printf("Running on GPU %d: %s\n", device, prop.name);
    
    char *cuda_seq1, *cuda_seq2;
    int *cuda_score;
    int *cuda_max_i, *cuda_max_j, *cuda_max_score;
    char *cuda_aligned_seq1, *cuda_aligned_seq2;
    int *cuda_align_length;

    // Allocate memory on the device
    cudaMalloc((void **)&cuda_seq1, size1 * sizeof(char));
    cudaMalloc((void **)&cuda_seq2, size2 * sizeof(char));
    cudaMalloc((void **)&cuda_score, (size1 + 1) * (size2 + 1) * sizeof(int));
    cudaMalloc((void **)&cuda_max_i, sizeof(int));
    cudaMalloc((void **)&cuda_max_j, sizeof(int));
    cudaMalloc((void **)&cuda_max_score, sizeof(int));
    
    // Allocate memory for alignment results (assuming max length)
    size_t max_align_len = size1 + size2;  // Worst case
    cudaMalloc((void **)&cuda_aligned_seq1, (max_align_len + 1) * sizeof(char));
    cudaMalloc((void **)&cuda_aligned_seq2, (max_align_len + 1) * sizeof(char));
    cudaMalloc((void **)&cuda_align_length, sizeof(int));

    // Initialize max values
    cudaMemset(cuda_max_i, 0, sizeof(int));
    cudaMemset(cuda_max_j, 0, sizeof(int));
    cudaMemset(cuda_max_score, 0, sizeof(int));
    
    // Copy sequences to device
    cudaMemcpy(cuda_seq1, seq1, size1 * sizeof(char), cudaMemcpyHostToDevice);
    cudaMemcpy(cuda_seq2, seq2, size2 * sizeof(char), cudaMemcpyHostToDevice);
    cudaMemset(cuda_score, 0, (size1 + 1) * (size2 + 1) * sizeof(int));

    // Fill score matrix in wave-front (anti-diagonal order)
    int total_diagonals = size1 + size2 - 1;
    int threads_per_block = 1024; 
    int shared_mem_size = threads_per_block * sizeof(int);

    for (int diag = 1; diag <= total_diagonals; ++diag) {
        int elements_in_diag = min(diag, min(static_cast<int>(size1), static_cast<int>(size2)));
        int blocks = (elements_in_diag + threads_per_block - 1) / threads_per_block;

        smith_waterman_kernel_optimized<<<blocks, threads_per_block, shared_mem_size>>>(
            cuda_seq1, cuda_seq2, cuda_score,
            size1, size2, diag);
        
        cudaDeviceSynchronize();
    }

    // Find maximum score and position using GPU
    int num_blocks = min(32, (int)((size1 * size2 + threads_per_block - 1) / threads_per_block));
    int find_max_shared_mem = threads_per_block * 3 * sizeof(int); // For scores, i indices, and j indices
    
    find_max_score_kernel<<<num_blocks, threads_per_block, find_max_shared_mem>>>(
        cuda_score, cuda_max_i, cuda_max_j, cuda_max_score, 
        size1, size2);
    
    cudaDeviceSynchronize();
    
    // Get max values from device
    int max_i, max_j, max_score;
    cudaMemcpy(&max_i, cuda_max_i, sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(&max_j, cuda_max_j, sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(&max_score, cuda_max_score, sizeof(int), cudaMemcpyDeviceToHost);
    
    // Perform traceback on GPU
    traceback_kernel<<<1, 1>>>(
        cuda_seq1, cuda_seq2, cuda_score,
        max_i, max_j, size1, size2,
        cuda_aligned_seq1, cuda_aligned_seq2, cuda_align_length);
    
    cudaDeviceSynchronize();
    
    // Copy alignment results back to host
    int align_length;
    cudaMemcpy(&align_length, cuda_align_length, sizeof(int), cudaMemcpyDeviceToHost);
    
    std::vector<char> h_aligned_seq1(align_length + 1);
    std::vector<char> h_aligned_seq2(align_length + 1);
    
    cudaMemcpy(h_aligned_seq1.data(), cuda_aligned_seq1, (align_length + 1) * sizeof(char), cudaMemcpyDeviceToHost);
    cudaMemcpy(h_aligned_seq2.data(), cuda_aligned_seq2, (align_length + 1) * sizeof(char), cudaMemcpyDeviceToHost);
    
    // Free device memory
    cudaFree(cuda_seq1);
    cudaFree(cuda_seq2);
    cudaFree(cuda_score);
    cudaFree(cuda_max_i);
    cudaFree(cuda_max_j);
    cudaFree(cuda_max_score);
    cudaFree(cuda_aligned_seq1);
    cudaFree(cuda_aligned_seq2);
    cudaFree(cuda_align_length);

    // Convert to strings
    std::string alignedSeq1(h_aligned_seq1.data(), align_length);
    std::string alignedSeq2(h_aligned_seq2.data(), align_length);
    
    return {alignedSeq1, alignedSeq2};
}
