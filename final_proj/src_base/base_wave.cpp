#include <algorithm>
#include <iostream>
#include <bits/stdc++.h>
#include "base_main.hpp"
#include "../defines.hpp"
#include <omp.h>

#define CHECK_CORE
using namespace std;

//start and end are inclusive
std::tuple<int, int, int> process_block(int start_i, int end_i, int start_j, int end_j, 
                   std::vector<std::vector<int>>& matrix, const std::string& seq1, const std::string& seq2) {

    int maxScore = 0;
    int maxI = 0;
    int maxJ = 0;

    for (size_t i = start_i; i <= end_i; ++i)
    {
        for (size_t j = start_j; j <= end_j; ++j)
        {
            
            int matchScore = (seq1[i - 1] == seq2[j - 1]) ? MATCH_SCORE : MISMATCH_SCORE;
            matrix[i][j] = std::max({0,
                                    matrix[i - 1][j - 1] + matchScore,
                                    matrix[i - 1][j] + GAP_SCORE,
                                    matrix[i][j - 1] + GAP_SCORE});
            if (matrix[i][j] > maxScore)
            {
                maxScore = matrix[i][j];
                maxI = i;
                maxJ = j;
            }
        }
    }
    return std::make_tuple(maxScore, maxI, maxJ);
}

std::pair<std::string, std::string> smithWaterman(const char *seq1, size_t size1, const char *seq2, size_t size2) {

    //MATRIX ALLOCATION + TIMING HARNESS
    std::vector<std::vector<int>> score(size1 + 1, std::vector<int>(size2 + 1, 0));

    int maxScore = 0;
    int maxI = 0, maxJ = 0;
    std::tuple<int, int, int> block_out;
    int num_blocks_seq1 = (size1 + BASELINE_TILE_DIM - 1) / BASELINE_TILE_DIM;
    //includes irregularly shaped blocks
    int num_blocks_seq2 = (size2 + BASELINE_TILE_DIM - 1) / BASELINE_TILE_DIM;


    //below, k, block_num_x, block_num_y start at 0
    //k is the number of the anti-diagonal
    //it is also the sum of the indexes of a block in it (if square)
    for (int k = 0; k < num_blocks_seq1 + num_blocks_seq2 - 1; ++k) {
                                            // -2 since its 0 indexed
        // is going through the diagonal
        // block_num_x + block_num_y = k

        // x is in i direction
        // y is in j direction
        // the matrix is j major (column major)
        // thesse diagonals are going up and to the right
        #pragma omp parallel for schedule(dynamic)
        for (int block_num_x = 0; block_num_x <= k; ++block_num_x) {
            int block_num_y = k - block_num_x;
            // Check if the bounds
            // diagonals go up right, so need to check negative y, overbound x, and overbound y
            if (block_num_y >= 0 && block_num_x < num_blocks_seq1 && block_num_y < num_blocks_seq2) {
                
                #ifdef CHECK_CORE
                    int tid = omp_get_thread_num();
                    int cpu = sched_getcpu();
                    printf("Thread %d is running on CPU %d\n", tid, cpu);
                #endif

                int start_i = block_num_x * BASELINE_TILE_DIM + 1;
                int start_j = block_num_y * BASELINE_TILE_DIM + 1;
                int end_i = min(start_i + BASELINE_TILE_DIM - 1, (int)size1);
                int end_j = min(start_j + BASELINE_TILE_DIM - 1, (int)size2);
                // std::cout << start_i << "_" << end_i << "|" << start_j << "_" << end_j << " num_x = " << block_num_x << " num_y = " << block_num_y << std::endl;
                block_out = process_block(start_i, end_i, start_j, end_j, score, seq1, seq2);
                if (std::get<0>(block_out) > maxScore) {
                    #pragma omp critical
                    {
                        maxScore = std::get<0>(block_out);
                        maxI = std::get<1>(block_out);
                        maxJ = std::get<2>(block_out);
                    }
                }
            }
        }
        // std::cout << std::endl;  // New line for each anti-diagonal
    }

    //backtrack to find the aligned sequences
    std::string alignedSeq1, alignedSeq2;
    size_t i = maxI, j = maxJ;
    //BACKTRACKING
    while (i > 0 && j > 0 && score[i][j] > 0)
    {
        if (score[i][j] == score[i - 1][j - 1] + ((seq1[i - 1] == seq2[j - 1]) ? MATCH_SCORE : MISMATCH_SCORE))
        {
            alignedSeq1 += seq1[i - 1];
            alignedSeq2 += seq2[j - 1];
            i--;
            j--;
        }
        else if (score[i][j] == score[i - 1][j] + GAP_SCORE)
        {
            alignedSeq1 += seq1[i - 1];
            alignedSeq2 += '-';
            i--;
        }
        else // score[i][j] == score[i][j - 1] + GAP_SCORE
        {
            alignedSeq1 += '-';
            alignedSeq2 += seq2[j - 1];
            j--;
        }
    }

    //reverse sequences
    std::reverse(alignedSeq1.begin(), alignedSeq1.end());
    std::reverse(alignedSeq2.begin(), alignedSeq2.end());

    return {alignedSeq1, alignedSeq2}; // Return the aligned sequences
}