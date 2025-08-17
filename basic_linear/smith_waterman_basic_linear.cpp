#include "smith_waterman_basic_linear.hpp"
#include <algorithm>
#include <hls_stream.h>
#include <ap_int.h>

#define MAX_SIZE 32 // Adjust based on FPGA constraints

extern "C" void smith_waterman_basic_linear(
    const char seq1[MAX_SIZE], const char seq2[MAX_SIZE], const int seqsize[2],
    char alignedSeq1[MAX_SIZE], char alignedSeq2[MAX_SIZE]) 
{
    #pragma HLS INTERFACE m_axi port=seq1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=seq2 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=seqsize offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=alignedSeq1 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=alignedSeq2 offset=slave bundle=gmem4
    
    #pragma HLS INTERFACE s_axilite port=seq1 bundle=control
    #pragma HLS INTERFACE s_axilite port=seq2 bundle=control
    #pragma HLS INTERFACE s_axilite port=seqsize bundle=control
    #pragma HLS INTERFACE s_axilite port=alignedSeq1 bundle=control
    #pragma HLS INTERFACE s_axilite port=alignedSeq2 bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    const int match = 2;
    const int mismatch = -1;
    const int gap = -1;

    // Static score matrix (fixed size to comply with HLS constraints)
    int score[MAX_SIZE][MAX_SIZE] = {0};

    #pragma HLS ARRAY_PARTITION variable=score complete dim=2
    #pragma HLS BIND_STORAGE variable=score type=RAM_2P impl=BRAM
    //https://docs.amd.com/r/en-US/ug1399-vitis-hls/pragma-HLS-bind_storage

    int maxScore = 0;
    int maxI = 0, maxJ = 0;

    // **FILL SCORE MATRIX**
    for (int i = 1; i <= seqsize[0]; i++) {
        #pragma HLS PIPELINE
        for (int j = 1; j <= seqsize[1]; j++) {
            int matchScore = (seq1[i - 1] == seq2[j - 1]) ? match : mismatch;
            score[i][j] = std::max(
                                    0,
                                    std::max(score[i - 1][j - 1] + matchScore,
                                    std::max(score[i - 1][j] + gap, score[i][j - 1] + gap))
            );
            if (score[i][j] > maxScore) {
                maxScore = score[i][j];
                maxI = i;
                maxJ = j;
            }
        }
    }

    // **BACKTRACKING**
    int i = maxI, j = maxJ;
    int idx = 0;
    while (i > 0 && j > 0 && score[i][j] > 0 && idx < MAX_SIZE - 1) {
        #pragma HLS PIPELINE
        if (seq1[i - 1] == seq2[j - 1]) {
            alignedSeq1[idx] = seq1[i - 1];
            alignedSeq2[idx] = seq2[j - 1];
            i--;
            j--;
        } else if (score[i][j] == score[i - 1][j] + gap) {
            alignedSeq1[idx] = seq1[i - 1];
            alignedSeq2[idx] = '-';
            i--;
        } else {
            alignedSeq1[idx] = '-';
            alignedSeq2[idx] = seq2[j - 1];
            j--;
        }
        idx++;
    }

    // Null-terminate the aligned sequences
    alignedSeq1[idx] = '\0';
    alignedSeq2[idx] = '\0';

    // **REVERSE STRINGS IN PLACE** (Since we backtrack in reverse)
    for (int k = 0; k < idx / 2; k++) {
        #pragma HLS UNROLL
        char temp = alignedSeq1[k];
        alignedSeq1[k] = alignedSeq1[idx - k - 1];
        alignedSeq1[idx - k - 1] = temp;

        temp = alignedSeq2[k];
        alignedSeq2[k] = alignedSeq2[idx - k - 1];
        alignedSeq2[idx - k - 1] = temp;
    }
}