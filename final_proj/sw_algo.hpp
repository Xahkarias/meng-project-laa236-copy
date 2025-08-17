#ifndef SW_ALGORITHM_HPP
#define SW_ALGORITHM_HPP

#include <string>
#include <utility>

extern "C" void SW_basic_linear(
    char* seq1, char* seq2, volatile int* buffer, //buffer is scaled to be the tilenum[0] tiles wide and tilenum[1] tiles tall 
        //seq1 and seq2 are actually len+1, since null terminator is included
    const int seqsize[2], const int tilenum[2], 
        //0 = seq1_len   //0 = ceil(seq1_len/TILE_SIZE)
        //1 = seq2_len   //1 = ceil(seq2_len/TILE_SIZE)
    char* alignedSeq1, char* alignedSeq2); 
        //maximum length of these is seq1_len+seq2_len (including null term)

#endif // SW_ALGORITHM_HPP