#include <iostream>
#include <cstring>
#include "smith_waterman_basic_linear.hpp"

#define MAX_SIZE 32

int main() {
    // Define test sequences
    const char seq1[MAX_SIZE] = "AGCTGAC";
    const char seq2[MAX_SIZE] = "GCTAGAC";

    // Expected sequence lengths
    int seqsize[2] = {7, 7};

    // Output buffers for aligned sequences
    char alignedSeq1[MAX_SIZE] = {0};
    char alignedSeq2[MAX_SIZE] = {0};

    // Call the HLS function
    smith_waterman_basic_linear(seq1, seq2, seqsize, alignedSeq1, alignedSeq2);

    // Print results
    std::cout << "Sequence 1: " << seq1 << std::endl;
    std::cout << "Sequence 2: " << seq2 << std::endl;
    std::cout << "Aligned 1 : " << alignedSeq1 << std::endl;
    std::cout << "Aligned 2 : " << alignedSeq2 << std::endl;

    // Simple verification (basic check)
    bool pass = (strlen(alignedSeq1) > 0 && strlen(alignedSeq2) > 0);

    if (pass) {
        std::cout << "Test Passed!" << std::endl;
        return 0; // Success
    } else {
        std::cout << "Test Failed!" << std::endl;
        return 1; // Failure
    }
}