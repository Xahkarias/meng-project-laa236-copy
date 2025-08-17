#include "POA_basic_linear.hpp"
#include <cstring>
#include <ostream>
#include <iostream>

//#define DEBUG
//#define DEBUG_MORE

extern "C" void POA_basic_linear(
    POAGraph &graph, const char seq[MAX_SEQ_LENGTH], const int seqSize[1],
    char alignedSeq[MAX_SEQ_LENGTH+1], char alignedInputSeq[MAX_SEQ_LENGTH], int alignSeqSize[1]) 
    //+1 in order to handle max seq length
{
    //assume that graphs are sorted
    //-------------------------------------------------------------------------------------
    int score[MAX_SEQ_LENGTH + 1][MAX_NODES + 1] = {0};
    #pragma HLS ARRAY_PARTITION variable=score complete dim=1
        //sequence at the top, graph on left
        //this partitions each row into its own memory access
    int backtrack_node[MAX_SEQ_LENGTH + 1][MAX_NODES + 1] = {0};
    #pragma HLS ARRAY_PARTITION variable=backtrack_node complete dim=1
        //same size stuff as above
        //tracks which node to backtrack to
    bool backtrack_gap[MAX_SEQ_LENGTH + 1][MAX_NODES + 1] = {0};
    #pragma HLS ARRAY_PARTITION variable=backtrack_gap complete dim=1
        //tracks if the index is straight up (true), or slightly diagonal (false)
        //this is false if it is left, but you should check the node number first

    //-------------------------------------------------------------------------------------
    //for each character (go right)
    for (int seqcharPlus1 = 1; seqcharPlus1 <= seqSize[0]; seqcharPlus1++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=MAX_SEQ_LENGTH
        //#pragma HLS UNROLL factor=4
        //does not like unrolling, probably due to data dependencies
        #pragma HLS PIPELINE
        //cant pipeline and dataflow
        
        //for each node (go down)
        //this is presorted to topological order, so dont need to worry
        for (int nodeNum = 0; nodeNum < graph.numNodes; nodeNum++) {
            #pragma HLS LOOP_TRIPCOUNT min=1 max=MAX_NODES
            //putting a pipeline here increases runtime
        
            
            //variables for this 
            int bestScore = 0;
            int bestPrevNode = -1;
            bool bestIsGap = false;

            // Check each incoming edge
            if (graph.nodeNumEdges[nodeNum] > 0 ) {
                for (int edgeNum = 0; edgeNum < graph.nodeNumEdges[nodeNum]; edgeNum++) {
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=MAX_EDGES_PER_NODE
                    #pragma HLS PIPELINE
                    //currently the node we are going to fill is score[seqcharPlus1][nodeNum]
    
                    //need to check diagonal                    
                    int targNode = graph.nodeIncomingEdges[edgeNum][nodeNum];
                    int currValue = score[seqcharPlus1-1][targNode+1];
                                              //the diagonal node connected with current edge
                    int matchScore = graph.nodeLabels[nodeNum] == seq[seqcharPlus1-1] ? MATCH_SCORE : MISMATCH_SCORE;
                    #ifdef DEBUG_MORE
                    std::cout <<"DIAG || NodeNum: " << nodeNum << " seqchar: " << seqcharPlus1 - 1 << " | TargNode: " << targNode << " | matchScore:" << matchScore << " currValue: " << currValue << " edgenum: " << edgeNum << std::endl;
                    #endif
                    if (matchScore + currValue > bestScore) {
                        bestScore = matchScore + currValue;
                        bestPrevNode = targNode;
                        bestIsGap = false;
                    }
    
                    //need to check up gap
                    int currValueUp = score[seqcharPlus1][targNode+1];
                                              //above node connected with current edge
                    #ifdef DEBUG_MORE
                    //std::cout <<"ABVE || NodeNum: " << nodeNum << " TargNode: " << targNode << " | seqchar: " << seqcharPlus1 << " | currValue: " << currValueUp << " edgenum: " << edgeNum << std::endl;
                    #endif
                    if (GAP_SCORE + currValueUp > bestScore) {
                        bestScore = GAP_SCORE + currValueUp;
                        bestPrevNode = targNode;
                        bestIsGap = true;
                    }
                }
            } 
            else { //this is the case where the node has no inputs, only outputs
                //only need to check mismatch
                int matchScore = graph.nodeLabels[nodeNum] == seq[seqcharPlus1-1] ? MATCH_SCORE : MISMATCH_SCORE;
                #ifdef DEBUG_MORE
                std::cout <<"HDSS || NodeNum: " << nodeNum << " TargNode: HL| seqchar: " << seqcharPlus1 - 1 << " | matchScore:" << matchScore << std::endl;
                #endif
                if (matchScore > bestScore) {
                    bestScore = matchScore;
                    bestPrevNode = -1; //lets hope this doesnt brick lmao
                    bestIsGap = false;
                }
            }

            
            //check left
            int currValue = score[seqcharPlus1-1][nodeNum+1];
            #ifdef DEBUG_MORE
            //std::cout <<"LEFT || NodeNum: " << nodeNum << " TargNode: " << nodeNum << " | seqchar: " << seqcharPlus1-1 << " | currValue: " << currValue << std::endl;
            #endif
            if (GAP_SCORE + currValue > bestScore) {
                bestScore = GAP_SCORE + currValue;
                bestPrevNode = nodeNum;
                bestIsGap = false;
            }

            //actually write into scoring
            score[seqcharPlus1][nodeNum+1] = bestScore;
            backtrack_node[seqcharPlus1][nodeNum+1] = bestPrevNode;
            backtrack_gap[seqcharPlus1][nodeNum+1] = bestIsGap;
        }
    }

    //-------------------------------------------------------------------------------------
    //finding the max of the entire matrix
    //probably could put into the loop above but later problem
    
    int maxVal = 0;
    int maxNode = 0;
    int maxSeqPos = 0;

    for (int i = 0; i <= seqSize[0]; i++) {
        for (int j = 0; j <= graph.numNodes; j++) {
            if (score[i][j] > maxVal) {
                maxVal = score[i][j];
                maxNode = j-1;
                maxSeqPos = i-1;
            }
        }
    }

    //-------------------------------------------------------------------------------------
    //int score[MAX_SEQ_LENGTH + 1][MAX_NODES + 1] = {0};
        //sequence at the top, graph on left

    #ifdef DEBUG
    // [MAX_SEQ_LENGTH + 1][MAX_NODES + 1]
    std::cout << "Scores:\n----- ";
    for (int i = 0; i < seqSize[0]; i++) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    for (int j = 0; j <= graph.numNodes; j++) {
        if (j == 0) {
            std::cout << "--- ";
        } else {
            std::cout << j-1 << "-" << graph.nodeLabels[j-1] << " ";
        }
        for (int i = 0; i <= seqSize[0]; i++) {
            std::cout << score[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "Max Score: " << maxVal << " | Max Node: " << maxNode << " | Max SeqChar: " << maxSeqPos << std::endl;
    #endif
    #ifdef DEBUG_MORE
    std::cout << "Backtrack Node:\n";
    for (int j = 0; j <= graph.numNodes; j++) {
        for (int i = 0; i <= seqSize[0]; i++) {
            std::cout << backtrack_node[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "Backtrack IsGap:\n";
    for (int j = 0; j <= graph.numNodes; j++) {
        for (int i = 0; i <= seqSize[0]; i++) {
            std::cout << backtrack_gap[i][j] << " ";
        }
        std::cout << std::endl;
    }
    #endif
    //-------------------------------------------------------------------------------------

    // traceback
    int tracebackPos = maxSeqPos;
    int tracebackNode = maxNode;
    int alignedIdx = 0;
    bool keepGoing = true;
    
    while (tracebackPos >= 0 && tracebackNode >= 0 && keepGoing) {
        // Get previous node and position
        int prevNode = backtrack_node[tracebackPos + 1][tracebackNode + 1];
        
        if (score[tracebackPos][prevNode+1] == 0) {
            #ifdef DEBUG_MORE
            std::cout << "EOL || ";
            #endif
            keepGoing = false;
        }

    
        #ifdef DEBUG
        std::cout << "Traceback Node / Pos: " << tracebackNode << " , " << tracebackPos 
                  << " | Value: " << score[tracebackPos+1][tracebackNode+1]
                  << " | Prev Node: " << prevNode 
                  << " | Gap: " << backtrack_gap[tracebackPos+1][tracebackNode+1] 
                  << std::endl;
        #endif
    
        // move left
        if (prevNode == tracebackNode) {
            //means that the seq should add a char
            alignedSeq[alignedIdx] = '-';
            alignedInputSeq[alignedIdx] = seq[tracebackPos];
            tracebackPos--;
        }  
        //move up
        else if (backtrack_gap[tracebackPos+1][tracebackNode+1]) {
            //means that the graph should add a char
            alignedSeq[alignedIdx] = graph.nodeLabels[tracebackNode];
            alignedInputSeq[alignedIdx] = '-';
            tracebackNode = prevNode;
        } 
        //diagonal
        else { 
            //add both
            alignedSeq[alignedIdx] = graph.nodeLabels[tracebackNode];
            alignedInputSeq[alignedIdx] = seq[tracebackPos];
            tracebackNode = prevNode;
            tracebackPos--;
        } 
        alignedIdx++;
    
        
    }

    for (int i = 0; i < alignedIdx / 2; i++) {
        std::swap(alignedSeq[i], alignedSeq[alignedIdx - i - 1]);
        std::swap(alignedInputSeq[i], alignedInputSeq[alignedIdx - i - 1]);
    }
    
    alignSeqSize[0] = alignedIdx;
}