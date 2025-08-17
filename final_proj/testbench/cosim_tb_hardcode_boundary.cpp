#include <cstdlib>
#include <iostream>
#include <cstring>
#include "../defines.hpp"
#include "../sw_algo.hpp"
#include <algorithm>
#include <cmath>

//rounds value up to nearest multiple of x
static int ceilToMultiple(int value, int x) {
    return ((value + x - 1) / x) * x;
}

static int numTiles(int value, int x) {
    return std::ceil((value + x - 1) / x);
}

//this is hardcoded because cosim is a pain and refuses to use stuff like fstream without crapping out
int main() {
    ///////////////////////////////////////////////////////////////////////////////////////
    // EXPECTED OUTPUT SECTION - Modify these values to match expected results
    ///////////////////////////////////////////////////////////////////////////////////////
    const char* expected_aligned_seq1 = "ACACC-AGTACCCAA---A-ACCAGGC";
    const char* expected_aligned_seq2 = "ACACCTA-TACCCAAGGCATAACA-GC";
    ///////////////////////////////////////////////////////////////////////////////////////
    
    // Define input, includes null character
    ///////////////////////////////////////////////////////////////////////////////////////
    int seqsize[2] = {44, 30}; // Original sequence lengths
    ///////////////////////////////////////////////////////////////////////////////////////
   
    // Calculate padded sizes and number of tiles
    int inputsize1 = ceilToMultiple(seqsize[0], TILE_DIMENSION) + 1;
    int inputsize2 = ceilToMultiple(seqsize[1], TILE_DIMENSION) + 1;
    int numTilesVar[2] = {
        numTiles(seqsize[0], TILE_DIMENSION), // Number of horizontal tiles
        numTiles(seqsize[1], TILE_DIMENSION)  // Number of vertical tiles
    };
   
    std::cout << "Input sizes: " << inputsize1 << " || " << inputsize2 << std::endl;
    std::cout << "Tile counts: " << numTilesVar[0] << " || " << numTilesVar[1] << std::endl;
   
    // Allocate memory for sequences
    char* seq1 = (char*)malloc(inputsize1 * sizeof(char));
    char* seq2 = (char*)malloc(inputsize2 * sizeof(char));
   
    ///////////////////////////////////////////////////////////////////////////////////////
    strcpy(seq1, "ACACCAGTACCCAAAACCAGGCGGGCTCGCCACGTCGGCTAATC");
    strcpy(seq2, "ACACCTATACCCAAGGCATAACAGCCAATC");
    ///////////////////////////////////////////////////////////////////////////////////////
    if (seq1 == nullptr || seq2 == nullptr) {
        return 59;
    }
   
    // Output buffers for aligned sequences
    char* alignedSeq1 = (char*)malloc((seqsize[0]+seqsize[1]) * sizeof(char));
    char* alignedSeq2 = (char*)malloc((seqsize[0]+seqsize[1]) * sizeof(char));
   
    // Pointer for score buffer - size needs to include the boundary cells
    int buffer_horz_size = numTilesVar[0] * (TILE_DIMENSION + 1) * 2;
    int buffer_vert_size = numTilesVar[1];
    int* buffer = (int*)malloc(sizeof(int) * buffer_horz_size * buffer_vert_size);
   
    // Print input
    std::cout << "Sequence 1: " << seq1 << std::endl;
    std::cout << "Sequence 2: " << seq2 << std::endl;
   
    // Call the HLS function
    SW_basic_linear(seq1, seq2, buffer, seqsize, numTilesVar, alignedSeq1, alignedSeq2);
   
    // Reverse the aligned sequences
    std::reverse(alignedSeq1, alignedSeq1 + strlen(alignedSeq1));
    std::reverse(alignedSeq2, alignedSeq2 + strlen(alignedSeq2));
   
    // Print results
    std::cout << "Aligned 1 : " << alignedSeq1 << std::endl;
    std::cout << "Aligned 2 : " << alignedSeq2 << std::endl;
   
    // Verification against expected outputs
    bool pass = true;
    if (strcmp(alignedSeq1, expected_aligned_seq1) != 0) {
        std::cout << "ERROR: Aligned sequence 1 does not match expected output" << std::endl;
        std::cout << "Expected: " << expected_aligned_seq1 << std::endl;
        std::cout << "Got     : " << alignedSeq1 << std::endl;
        pass = false;
    }
    
    if (strcmp(alignedSeq2, expected_aligned_seq2) != 0) {
        std::cout << "ERROR: Aligned sequence 2 does not match expected output" << std::endl;
        std::cout << "Expected: " << expected_aligned_seq2 << std::endl;
        std::cout << "Got     : " << alignedSeq2 << std::endl;
        pass = false;
    }
   
    std::free(seq1);
    std::free(seq2);
    std::free(alignedSeq1);
    std::free(alignedSeq2);
    std::free(buffer);
   
    if (pass) {
        std::cout << "Test Passed! Output matches expected results." << std::endl;
        return 0;
    } else {
        std::cout << "Test Failed! Output does not match expected results." << std::endl;
        return 1;
    }
}