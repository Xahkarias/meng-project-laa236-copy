#include <iostream>
#include <cstring>
#include <queue>
#include <algorithm>
#include "hls_stream.h"
#include "POA_basic_linear.hpp"

void topologicalSort(POAGraph &graph) {
    int inDegree[MAX_NODES] = {0};
    std::queue<int> q;
    int sortedIndex = 0;
    int sortedNodes[MAX_NODES];
    POAGraph tempGraph = graph;

    // Compute in-degree of each node
    for (int i = 0; i < graph.numNodes; i++) {
        for (int j = 0; j < graph.nodeNumEdges[i]; j++) {
            inDegree[graph.nodeIncomingEdges[j][i]]++;
        }
    }

    // Enqueue nodes with in-degree 0
    for (int i = 0; i < graph.numNodes; i++) {
        if (inDegree[i] == 0) {
            q.push(i);
        }
    }

    while (!q.empty()) {
        int node = q.front();
        q.pop();
        sortedNodes[sortedIndex++] = node;

        for (int j = 0; j < graph.nodeNumEdges[node]; j++) {
            int neighbor = graph.nodeIncomingEdges[j][node];
            if (--inDegree[neighbor] == 0) {
                q.push(neighbor);
            }
        }
    }

    // Reverse the sortedNodes order
    std::reverse(sortedNodes, sortedNodes + sortedIndex);

    // Reorder the graph based on reversed sortedNodes
    POAGraph newGraph;
    newGraph.numNodes = graph.numNodes;
    for (int i = 0; i < graph.numNodes; i++) {
        int newIndex = sortedNodes[i];
        newGraph.nodeLabels[i] = tempGraph.nodeLabels[newIndex];
        newGraph.nodeNumEdges[i] = tempGraph.nodeNumEdges[newIndex];
        for (int j = 0; j < tempGraph.nodeNumEdges[newIndex]; j++) {
            newGraph.nodeIncomingEdges[j][i] = tempGraph.nodeIncomingEdges[j][newIndex];
        }
    }
    graph = newGraph;
}

void test_POA_basic_sw_20() {
    POAGraph graph;
    graph.numNodes = 20;
    graph.nodeLabels[0] = 'A';
    graph.nodeLabels[1] = 'F';
    graph.nodeLabels[2] = 'B';
    graph.nodeLabels[3] = 'C';
    graph.nodeLabels[4] = 'D';
    graph.nodeLabels[5] = 'E';
    graph.nodeLabels[6] = 'E';
    graph.nodeLabels[7] = 'E';
    graph.nodeLabels[8] = 'E';
    graph.nodeLabels[9] = 'E';
    graph.nodeLabels[10] = 'A';
    graph.nodeLabels[11] = 'F';
    graph.nodeLabels[12] = 'B';
    graph.nodeLabels[13] = 'C';
    graph.nodeLabels[14] = 'D';
    graph.nodeLabels[15] = 'E';
    graph.nodeLabels[16] = 'E';
    graph.nodeLabels[17] = 'E';
    graph.nodeLabels[18] = 'E';
    graph.nodeLabels[19] = 'E';

    graph.nodeNumEdges[0] = 0;
    graph.nodeNumEdges[1] = 1; graph.nodeIncomingEdges[0][1] = 0;
    graph.nodeNumEdges[2] = 1; graph.nodeIncomingEdges[0][2] = 1;
    graph.nodeNumEdges[3] = 1; graph.nodeIncomingEdges[0][3] = 2;
    graph.nodeNumEdges[4] = 1; graph.nodeIncomingEdges[0][4] = 3;
    graph.nodeNumEdges[5] = 1; graph.nodeIncomingEdges[0][5] = 4;
    graph.nodeNumEdges[6] = 1; graph.nodeIncomingEdges[0][6] = 5;
    graph.nodeNumEdges[7] = 1; graph.nodeIncomingEdges[0][7] = 6;
    graph.nodeNumEdges[8] = 1; graph.nodeIncomingEdges[0][8] = 7;
    graph.nodeNumEdges[9] = 1; graph.nodeIncomingEdges[0][9] = 8;
    graph.nodeNumEdges[10] = 1; graph.nodeIncomingEdges[0][10] = 9;
    graph.nodeNumEdges[11] = 1; graph.nodeIncomingEdges[0][11] = 10;
    graph.nodeNumEdges[12] = 1; graph.nodeIncomingEdges[0][12] = 11;
    graph.nodeNumEdges[13] = 1; graph.nodeIncomingEdges[0][13] = 12;
    graph.nodeNumEdges[14] = 1; graph.nodeIncomingEdges[0][14] = 13;
    graph.nodeNumEdges[15] = 1; graph.nodeIncomingEdges[0][15] = 14;
    graph.nodeNumEdges[16] = 1; graph.nodeIncomingEdges[0][16] = 15;
    graph.nodeNumEdges[17] = 1; graph.nodeIncomingEdges[0][17] = 16;
    graph.nodeNumEdges[18] = 1; graph.nodeIncomingEdges[0][18] = 17;
    graph.nodeNumEdges[19] = 1; graph.nodeIncomingEdges[0][19] = 18;

    // Perform topological sorting (modifies the graph directly)
    topologicalSort(graph);

    std::cout << "Topologically Sorted Nodes: ";
    for (int i = 0; i < graph.numNodes; i++) {
        std::cout << graph.nodeLabels[i] << " ";
    }
    std::cout << std::endl;

    char seq[MAX_SEQ_LENGTH] = "AFBCDEEEEEAFBCDEEEEE";
    int seqSize[1] = {20};
    char alignedSeq[MAX_SEQ_LENGTH] = {0};
    char alignedInputSeq[MAX_SEQ_LENGTH] = {0};
    int alignSeqSize[1] = {0};

    POA_basic_linear(graph, seq, seqSize, alignedSeq, alignedInputSeq, alignSeqSize);

    std::cout << "Aligned Graph Sequence: ";
    for (int i = 0; i < alignSeqSize[0]; i++) {
        std::cout << alignedSeq[i];
    }
    std::cout << std::endl;

    std::cout << "Aligned Input Sequence: ";
    for (int i = 0; i < alignSeqSize[0]; i++) {
        std::cout << alignedInputSeq[i];
    }
    std::cout << std::endl;

    std::cout << "Alignment Length: " << alignSeqSize[0] << std::endl;
}

int main() {
    test_POA_basic_sw_20();
    return 0;
}
