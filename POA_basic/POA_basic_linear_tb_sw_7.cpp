#include <iostream>
#include <cstring>
#include "hls_stream.h"
#include "POA_basic_linear.hpp"

void test_POA_basic_linear() {
    POAGraph graph;
    // graph.numNodes = 4;
    // graph.nodeLabels[0] = 'A';
    // graph.nodeLabels[1] = 'T';
    // graph.nodeLabels[2] = 'G';
    // graph.nodeLabels[3] = 'G';

    // graph.nodeNumEdges[0] = 0;
    // graph.nodeNumEdges[1] = 1; graph.nodeIncomingEdges[0][1] = 0;
    // graph.nodeNumEdges[2] = 1; graph.nodeIncomingEdges[0][2] = 1;
    // graph.nodeNumEdges[3] = 1; graph.nodeIncomingEdges[0][3] = 2;

    graph.numNodes = 7;
    graph.nodeLabels[0] = 'G';
    graph.nodeLabels[1] = 'G';
    graph.nodeLabels[2] = 'T';
    graph.nodeLabels[3] = 'T';
    graph.nodeLabels[4] = 'G';
    graph.nodeLabels[5] = 'A';
    graph.nodeLabels[6] = 'C';

    graph.nodeNumEdges[0] = 0;
    graph.nodeNumEdges[1] = 1; graph.nodeIncomingEdges[0][1] = 0;
    graph.nodeNumEdges[2] = 1; graph.nodeIncomingEdges[0][2] = 1;
    graph.nodeNumEdges[3] = 1; graph.nodeIncomingEdges[0][3] = 2;
    graph.nodeNumEdges[4] = 1; graph.nodeIncomingEdges[0][4] = 3;
    graph.nodeNumEdges[5] = 1; graph.nodeIncomingEdges[0][5] = 4;
    graph.nodeNumEdges[6] = 1; graph.nodeIncomingEdges[0][6] = 5;

    char seq[MAX_SEQ_LENGTH] = "TGTTAC";
    int seqSize[1] = {6};
    char alignedSeq[MAX_SEQ_LENGTH] = {0};
    char alignedInputSeq[MAX_SEQ_LENGTH] = {0};
    int alignSeqSize[1] = {0};

    POA_basic_linear(graph, seq, seqSize, alignedSeq, alignedInputSeq, alignSeqSize);

    std::cout << "Aligned Graph Sequence: ";
    for (int i = 0; i < alignSeqSize[0]; i++) {
        std::cout << alignedSeq[i];
    }
    std::cout << std::endl << "Aligned Input Sequence: ";
    for (int i = 0; i < alignSeqSize[0]; i++) {
        std::cout << alignedInputSeq[i];
    }
    std::cout << "\nAlignment Length: " << alignSeqSize[0] << std::endl;
}

int main() {
    test_POA_basic_linear();
    return 0;
}