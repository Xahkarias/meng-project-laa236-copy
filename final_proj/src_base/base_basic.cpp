#include <algorithm>
#include <iostream>
#include <bits/stdc++.h>
#include "base_main.hpp"
#include "../defines.hpp"

using namespace std;

//simple blocked implementation
//written in a lab for a previous class

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

    //MATRIX ALLOCATION
    std::vector<std::vector<int>> score(size1 + 1, std::vector<int>(size2 + 1, 0));

    int maxScore = 0;
    int maxI = 0, maxJ = 0;
    std::tuple<int, int, int> block_out;

    //PROCESSING
    for (size_t start_i = 1; start_i < size1; start_i += BASELINE_TILE_DIM) {
        for (size_t start_j = 1; start_j < size2; start_j += BASELINE_TILE_DIM) {
            int end_i = min(start_i + BASELINE_TILE_DIM - 1, size1);
            int end_j = min(start_j + BASELINE_TILE_DIM - 1, size2);
            //std::cout << start_i << "_" << end_i << "|" << start_j << "_" << end_j << std::endl;
            block_out = process_block(start_i, end_i, start_j, end_j, score, seq1, seq2);
            if (std::get<0>(block_out) > maxScore) {
                maxScore = std::get<0>(block_out);
                maxI = std::get<1>(block_out);
                maxJ = std::get<2>(block_out);
            }
        }
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

    //reverse the aligned sequences
    std::reverse(alignedSeq1.begin(), alignedSeq1.end());
    std::reverse(alignedSeq2.begin(), alignedSeq2.end());

    return {alignedSeq1, alignedSeq2}; // Return the aligned sequences
}