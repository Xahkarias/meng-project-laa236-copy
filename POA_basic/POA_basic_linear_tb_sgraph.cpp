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

void test_POA_basic_linear() {
    POAGraph graph;
    graph.numNodes = 7;
    graph.nodeLabels[0] = 'A';
    graph.nodeLabels[1] = 'F';
    graph.nodeLabels[2] = 'B';
    graph.nodeLabels[3] = 'C';
    graph.nodeLabels[4] = 'D';
    graph.nodeLabels[5] = 'E';
    graph.nodeLabels[6] = 'E';

    graph.nodeNumEdges[0] = 0;
    graph.nodeNumEdges[1] = 0;
    graph.nodeNumEdges[2] = 2; graph.nodeIncomingEdges[0][2] = 0; graph.nodeIncomingEdges[1][2] = 1;
    graph.nodeNumEdges[3] = 1; graph.nodeIncomingEdges[0][3] = 2;
    graph.nodeNumEdges[4] = 1; graph.nodeIncomingEdges[0][4] = 3;
    graph.nodeNumEdges[5] = 1; graph.nodeIncomingEdges[0][5] = 3;
    graph.nodeNumEdges[6] = 2; graph.nodeIncomingEdges[0][6] = 4; graph.nodeIncomingEdges[1][6] = 5;

    // Perform topological sorting (modifies the graph directly)
    topologicalSort(graph);

    std::cout << "Topologically Sorted Nodes: ";
    for (int i = 0; i < graph.numNodes; i++) {
        std::cout << graph.nodeLabels[i] << " ";
    }
    std::cout << std::endl;

    char seq[MAX_SEQ_LENGTH] = "ABBDE";
    int seqSize[1] = {5};
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
    test_POA_basic_linear();
    return 0;
}
