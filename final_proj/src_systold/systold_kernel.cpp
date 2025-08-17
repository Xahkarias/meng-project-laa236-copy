#include "../defines.hpp"
#include "../sw_algo.hpp"
#include <algorithm>
#include <hls_stream.h>
#include <ap_int.h>
#include <iostream>
#include <cstring>

void seq_buffer_load(char seq1_tilebuffer[TILE_DIMENSION], char seq2_tilebuffer[TILE_DIMENSION], 
                     char* seq1, char* seq2, int seq1_tile, int seq2_tile) 
{
    int seq1_start = seq1_tile * TILE_DIMENSION;
    int seq2_start = seq2_tile * TILE_DIMENSION;
    
    charbuffer_loop: for (int i = 0; i < TILE_DIMENSION; i++) {
        #pragma HLS UNROLL
        seq1_tilebuffer[i] = seq1[i+seq1_start];
        seq2_tilebuffer[i] = seq2[i+seq2_start];
    } 
}

void boundary_fill(int score[TILE_DIMENSION+1][TILE_DIMENSION+1], int horz_tile_num, int vert_tile_num, 
                   volatile int* buffer, int buffer_horz_size, int leftSideBoundaryBuffer[TILE_DIMENSION]) 
{
    if (horz_tile_num == 0 || vert_tile_num == 0) {
        score[0][0] = 0;
    } else {
        score[0][0] = buffer[(vert_tile_num * TILE_DIMENSION) * buffer_horz_size +
                              horz_tile_num * TILE_DIMENSION];  //first line is row num
    }
    boundary_fill_loop: for (int i = 1; i <= TILE_DIMENSION; i++) {
        #pragma HLS UNROLL
        //SCORE FIRST COLUMN
        //if we are on the left side
        if (horz_tile_num == 0) {
            score[i][0] = 0;
            //set left col to 0
        } else {
            score[i][0] = leftSideBoundaryBuffer[i - 1];
        }

        //SCORE FIRST ROW
        //if we are on the top
        if (vert_tile_num == 0) {
            score[0][i] = 0;
            //set top row to 0
        } else {
            score[0][i] = buffer[(vert_tile_num * TILE_DIMENSION) * buffer_horz_size +
                                 horz_tile_num * TILE_DIMENSION + i]; //first line is row num
        }
    }
} 

void score_buffer_store(int score[TILE_DIMENSION+1][TILE_DIMENSION+1], int horz_tile_num, int vert_tile_num, 
                        volatile int* buffer, int buffer_horz_size, int leftSideBoundaryBuffer[TILE_DIMENSION]) 
{
    store_loop_outer: for (int i = 1; i <= TILE_DIMENSION; i++) {
        leftSideBoundaryBuffer[i-1] = score[i][TILE_DIMENSION];
        store_loop_inner: for (int j = 1; j <= TILE_DIMENSION; j++) {
            #pragma HLS UNROLL
            buffer[(vert_tile_num * TILE_DIMENSION + i) * buffer_horz_size +
                    horz_tile_num * TILE_DIMENSION + j] = score[i][j];
        }
    }
}  

void shift_right(char* array, int seqsize) {
    // Shift elements from end to start, discarding the last one
    memmove(&array[1], &array[0], seqsize * sizeof(char));
    array[0] = 0;
}

void PE(int vert_tile_num, int horz_tile_num, int rowID, //where am i
    hls::stream<int> &aboveSideIn, hls::stream<int> &downOut, //who do i talk to 
    int rowHead[TILE_DIMENSION+1], char* seq1, char seq2Char, //where is the data
    const int seqsize1, const int seqsize2, //how big is stuff
    int maxArrBuffer[2], int firstColDiag, int firstColLeft) //extras
{
    //#pragma HLS INLINE
    
    int maxind;
        //it needs to write to maxind, but not read
    int max = 0;
        //but it does need to know the previous max

    int left = firstColLeft; //take from score
    int diag = firstColDiag;

    main_PE_inner_loop: for (int i = 1; i <= TILE_DIMENSION; i++) {
        #pragma HLS PIPELINE

        int above = aboveSideIn.read();
        //if we are within seq2 and seq1
        if ((rowID + vert_tile_num * TILE_DIMENSION <= seqsize2) && (i + horz_tile_num * TILE_DIMENSION <= seqsize1)) {
            
            int matchScore = (seq2Char == seq1[i-1]) ? MATCH_SCORE : MISMATCH_SCORE;
                //get data

            int myScore = std::max(
                0,
                std::max(diag + matchScore,
                std::max(above + GAP_SCORE, left + GAP_SCORE))
            );
            //do score calc
            downOut.write(myScore);
            rowHead[i] = myScore;
            //write to row

            left = myScore;
            diag = above;
            //update internal scratchpad            

            if (myScore > max) {
                max = myScore;
                maxind = horz_tile_num * TILE_DIMENSION + i;
            }

        } else {
            //rowHead[i] = 0;
                //why write if invalid location
            downOut.write(0);
        }
    }
    maxArrBuffer[0] = maxind;
    maxArrBuffer[1] = max;
}

void PE_start(int rowHead[TILE_DIMENSION+1], hls::stream<int> &downOut) {
    #pragma HLS inline off
    //#pragma HLS INLINE
    PE_start_inner_loop: for (int i = 1; i <= TILE_DIMENSION; i++) {
        //#pragma HLS PIPELINE
        downOut.write(rowHead[i]);
    }
}

void PE_end(hls::stream<int> &aboveSideIn) {
    #pragma HLS INLINE off
    PE_end_inner_loop: for (int i = 1; i <= TILE_DIMENSION; i++) {
        //#pragma HLS PIPELINE off
        aboveSideIn.read();
    }
}

void systolic_loop(int score[TILE_DIMENSION+1][TILE_DIMENSION+1], char* seq1_buffer, char* seq2_buffer, const int seqsize1_buffer, const int seqsize2_buffer, 
                   int maxArrBuffer[TILE_DIMENSION][2], int vert_tile_num, int horz_tile_num, int firstColDiag[TILE_DIMENSION], int firstColLeft[TILE_DIMENSION]) 
{
    #pragma HLS INLINE off
    #pragma HLS DATAFLOW //disable_start_propagation
    hls::stream<int> streams[TILE_DIMENSION+1];
    #pragma HLS STREAM variable=streams depth=3 type=fifo
    #pragma HLS ARRAY_PARTITION variable=streams type=complete
    
    PE_start(score[0], streams[0]);
    //if i put this in its own function it fuckin crashes
    systolic_inner_loop: for (int i = 1; i <= TILE_DIMENSION; i++) {
        #pragma HLS UNROLL
        PE( vert_tile_num, horz_tile_num, i, 
            streams[i-1], streams[i],
            score[i], seq1_buffer, seq2_buffer[i-1],
                //this passes the correct row head pointer
            seqsize1_buffer, seqsize2_buffer,
            maxArrBuffer[i-1], firstColDiag[i-1], firstColLeft[i-1]); 
    }
    PE_end(streams[TILE_DIMENSION]);
}


//#define DEBUG
//#define DEBUG_SCORE
//#define DEBUG_BUFFER
//#define DEBUG_MAX
//#define DEBUG_BACKTRACK
//#define DEBUG_OUTPUT
//host does the reversing

//Input strings: +1 due to taking a possible null terminator
//Output string: padded with nulls, will always have at least 1 null terminator
extern "C" void SW_basic_linear(
    char* seq1, char* seq2, volatile int* buffer, //buffer is scaled to be the tilenum[0] tiles wide and tilenum[1] tiles tall 
        //seq1 and seq2 are actually ceiled to (nearest multiple of TILE_SIZE) + 1, since null terminator is included
    const int seqsize[2], const int tilenum[2], 
        //0 = seq1_len   //0 = ceil(seq1_len/TILE_SIZE)
        //1 = seq2_len   //1 = ceil(seq2_len/TILE_SIZE)
    char* alignedSeq1, char* alignedSeq2)
        //maximum length of these is seq1_len+seq2_len (includes null term)

    //seq1 is on top, seq2 is on left

    //buffer is flattened 2d array
    //buffer is row major since thats how C/C++ does it by default
{
    #pragma HLS INTERFACE m_axi port=seq1 offset=slave bundle=gmem0 depth=1024
    #pragma HLS INTERFACE s_axilite port=seq1 bundle=control
    #pragma HLS INTERFACE m_axi port=seq2 offset=slave bundle=gmem1 depth=1024
    #pragma HLS INTERFACE s_axilite port=seq2 bundle=control
    #pragma HLS INTERFACE m_axi port=buffer offset=slave bundle=gmem2 depth=1024
    #pragma HLS INTERFACE s_axilite port=buffer bundle=control
    #pragma HLS INTERFACE m_axi port=seqsize offset=slave bundle=gmem5 
    #pragma HLS INTERFACE s_axilite port=seqsize bundle=control
    #pragma HLS INTERFACE m_axi port=tilenum offset=slave bundle=gmem6 
    #pragma HLS INTERFACE s_axilite port=tilenum bundle=control
    #pragma HLS INTERFACE m_axi port=alignedSeq1 offset=slave bundle=gmem3 depth=128
    #pragma HLS INTERFACE s_axilite port=alignedSeq1 bundle=control
    #pragma HLS INTERFACE m_axi port=alignedSeq2 offset=slave bundle=gmem4 depth=128
    #pragma HLS INTERFACE s_axilite port=alignedSeq2 bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    //initalizing tile buffers
    char seq1_tilebuffer[TILE_DIMENSION];
    #pragma HLS ARRAY_PARTITION variable=seq1_tilebuffer complete
    char seq2_tilebuffer[TILE_DIMENSION];
    #pragma HLS ARRAY_PARTITION variable=seq2_tilebuffer complete

    //ceil of seqsize / TILEDIM
    int horz_tile_max = tilenum[0];
    int vert_tile_max = tilenum[1];

    //score matrix
    int score[TILE_DIMENSION+1][TILE_DIMENSION+1];
            //num ROW       //num COL
    #pragma HLS ARRAY_PARTITION variable=score complete dim=1
    //rows are partitioned
    #pragma HLS BIND_STORAGE variable=score type=RAM_2P impl=BRAM
    //https://docs.amd.com/r/en-US/ug1399-vitis-hls/pragma-HLS-bind_storage

    int outputsize = seqsize[0] + seqsize[1];


    int buffer_horz_size = horz_tile_max * TILE_DIMENSION + 1;
    int buffer_vert_size = vert_tile_max * TILE_DIMENSION + 1;

    int maxScore = 0;
    int maxI = 0, maxJ = 0;

    int leftSideBoundaryBuffer[TILE_DIMENSION];
    #pragma HLS ARRAY_PARTITION variable=leftSideBoundaryBuffer complete dim=0

    //down the tiles
    vert_tile_loop: for (int vert_tile_num = 0; vert_tile_num < vert_tile_max; vert_tile_num++) {
        
        //across the tiles
        horz_tile_loop: for (int horz_tile_num = 0; horz_tile_num < horz_tile_max; horz_tile_num++) {
            #pragma HLS LOOP_FLATTEN off
            
            //load the sequence data
            seq_buffer_load(seq1_tilebuffer, seq2_tilebuffer, seq1, seq2, horz_tile_num, vert_tile_num);
                //TODO: seq1 and seq2 should be another buffer level
            boundary_fill(score, horz_tile_num, vert_tile_num, buffer, buffer_horz_size, leftSideBoundaryBuffer);

            const int seqsize1_buffer = seqsize[0];
            const int seqsize2_buffer = seqsize[1];
                //have to rebuffer due to dataflow issues

            #ifdef DEBUG_SCORE
                std::cout << std::endl;
                std::cout << std::endl;
                std::cout << "seq1_tilebuffer: ";
                for (int i = 0; i < TILE_DIMENSION && seq1_tilebuffer[i] != '\0'; ++i) {
                    std::cout << seq1_tilebuffer[i];
                }
                std::cout << std::endl;

                std::cout << "seq2_tilebuffer: ";
                for (int i = 0; i < TILE_DIMENSION && seq2_tilebuffer[i] != '\0'; ++i) {
                    std::cout << seq2_tilebuffer[i];
                }
                std::cout << std::endl;
            #endif

            //since each PE needs the initial diagonal, they need to access the first index of the row above them.
            //but this messes up the dataflow pragma, since it enters the other guy's space.
            //so copy the first column of score into a buffer, EXCEPT for the upper left corner
            int firstColDiag[TILE_DIMENSION];
            int firstColLeft[TILE_DIMENSION];
            #pragma HLS ARRAY_PARTITION variable=firstColDiag complete
            #pragma HLS ARRAY_PARTITION variable=firstColLeft complete
                //needed to avoid dataflow error
            first_col_load: for (int i = 0; i < TILE_DIMENSION; i++) {
                #pragma HLS UNROLL
                firstColDiag[i] = score[i][0];
                firstColLeft[i] = score[i+1][0];
            }

            int maxArrBuffer[TILE_DIMENSION][2];
            #pragma HLS ARRAY_PARTITION variable=maxArrBuffer complete

            systolic_loop(score, seq1_tilebuffer, seq2_tilebuffer, seqsize1_buffer, seqsize2_buffer, maxArrBuffer, vert_tile_num, horz_tile_num, firstColDiag, firstColLeft);

            //move max arr from PE to the main storage
            for (int i = 0; i < TILE_DIMENSION; i++) {
                if (maxScore < maxArrBuffer[i][1]) {
                    maxScore = maxArrBuffer[i][1];
                    maxI = i + vert_tile_num * TILE_DIMENSION + 1;
                    maxJ = maxArrBuffer[i][0];
                }
            }            

            #ifdef DEBUG_SCORE
                for (int i = 0; i <= TILE_DIMENSION; i++) {
                    for (int j = 0; j <= TILE_DIMENSION; j++) {
                        std::cout << score[i][j] << " ";
                    }
                    std::cout << std::endl;  // Move to the next line after each row
                }
                std::cout << std::endl;
                // std::cout << "Max I: " << maxI << " | Max J: " << maxJ << " | Max Val: " << maxScore << std::endl;
                // std::cout << "Size Seq1: " << seqsize[0] << " | Seq2: " << seqsize[1] << std::endl;
            #endif

            score_buffer_store(score, horz_tile_num, vert_tile_num, buffer, buffer_horz_size, leftSideBoundaryBuffer);
            
        }
    }

    
    //flushing the output array
    for (int i = 0; i < outputsize; i++) {
        alignedSeq1[i] = 0;
        alignedSeq2[i] = 0;
    }

    //flushing the buffer edges
    for (int i = 0; i < buffer_horz_size; i++) {
        buffer[i] = 0;
    }
    for (int i = 0; i < buffer_vert_size; i++) {
        buffer[i * buffer_horz_size] = 0;
    }

    //backtracking
    //keeping it simple, may improve later
    //backtrack is simply loading each value one by one from the buffer
    int i = maxI;
    int j = maxJ;
    int idx = 0;

    #ifdef DEBUG_BUFFER
        for (int p = 0; p < buffer_vert_size; p++) {
            for (int q = 0; q < buffer_horz_size; q++) {
                std::cout << buffer[p * buffer_horz_size + q] << " ";
            }
            std::cout << std::endl;  // Move to the next line after each row
        }
        std::cout << "BUFFER" << std::endl;
        std::cout << "Max I: " << maxI << " | Max J: " << maxJ << " | Max Val: " << maxScore << std::endl;
        std::cout << "Size Seq1: " << seqsize[0] << " | Seq2: " << seqsize[1] << std::endl;
        std::cout << "Seq1 " << seq1 << " | Seq2 " << seq2 << std::endl;
    #endif
    
    //saves on the subtraction operations
    //cant buffer since unknown size
    shift_right(seq1, seqsize[0]+1);
    shift_right(seq2, seqsize[1]+1);

    backtrack_loop: while (i > 0 && j > 0 && buffer[i * buffer_horz_size + j] != 0) {
        #ifdef DEBUG_BACKTRACK
            std::cout << " |||  = " << ((seq1[j] == seq2[i]) ? MATCH_SCORE : MISMATCH_SCORE) << " dia  = " << buffer[(i-1) * buffer_horz_size + j-1] << " |  me " << buffer[i * buffer_horz_size + j] << std::endl;
        #endif
        if (buffer[i * buffer_horz_size + j] == (buffer[(i-1) * buffer_horz_size + j-1] + ((seq1[j] == seq2[i]) ? MATCH_SCORE : MISMATCH_SCORE))) {
            alignedSeq1[idx] = seq1[j];
            alignedSeq2[idx] = seq2[i];
            i--;
            j--;
        } else if (buffer[i * buffer_horz_size + j] == buffer[i * buffer_horz_size + j - 1] + GAP_SCORE) {
            alignedSeq1[idx] = seq1[j];
            alignedSeq2[idx] = '-';
            j--;
        } else { // buffer[i][j] == buffer[i][j-1] + GAP_SCORE   
            alignedSeq1[idx] = '-';
            alignedSeq2[idx] = seq2[i];
            i--;
        }
        #ifdef DEBUG_BACKTRACK
            std::cout << "idx = " << idx << " | s1 = " << alignedSeq1[idx] << " s2 = " << alignedSeq2[idx] << std::endl;
        #endif
        idx++;
    }

    #ifdef DEBUG_OUTPUT
        std::cout << "alignedSeq1: ";
        for (int i = 0; i <= idx; ++i) {
            std::cout << (int)alignedSeq1[i] << " ";
        }
        std::cout << std::endl;

        std::cout << "alignedSeq2: ";
        for (int i = 0; i <= idx; ++i) {
            std::cout << (int)alignedSeq2[i] << " ";
        }
        std::cout << std::endl;
    #endif
}