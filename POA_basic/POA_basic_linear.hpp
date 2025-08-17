#ifndef SMITH_WATERMAN_HPP
#define SMITH_WATERMAN_HPP

// Constants
#define MAX_NODES 128
#define MAX_EDGES_PER_NODE 6
#define MAX_SEQ_LENGTH 128
// wikipedia
// #define MATCH_SCORE 3
// #define MISMATCH_SCORE -3
// #define GAP_SCORE -2

//hand done
#define MATCH_SCORE 1
#define MISMATCH_SCORE -1
#define GAP_SCORE -2

typedef struct {
    char nodeLabels[MAX_NODES];
    int nodeIncomingEdges[MAX_EDGES_PER_NODE][MAX_NODES];
        //[what edge you are looking at][what node are you currently at] = node number on the other end of edge
    int nodeNumEdges[MAX_NODES];
    int numNodes;
} POAGraph;


extern "C" void POA_basic_linear(
    POAGraph &graph, const char seq[MAX_SEQ_LENGTH], const int seqSize[1],
    char alignedSeq[MAX_SEQ_LENGTH], char alignedInputSeq[MAX_SEQ_LENGTH], int alignSeqSize[1]);

#endif