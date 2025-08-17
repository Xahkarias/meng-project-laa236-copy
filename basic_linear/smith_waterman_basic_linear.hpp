#ifndef SMITH_WATERMAN_HPP
#define SMITH_WATERMAN_HPP

#define MAX_SIZE 256  // Adjust based on FPGA constraints

// Smith-Waterman function declaration
extern "C" void smith_waterman_basic_linear(
    const char seq1[MAX_SIZE], const char seq2[MAX_SIZE], const int seqsize[2],
    char alignedSeq1[MAX_SIZE], char alignedSeq2[MAX_SIZE]);

#endif // SMITH_WATERMAN_HPP