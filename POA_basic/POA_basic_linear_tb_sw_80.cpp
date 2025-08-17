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

void test_POA_basic_sw_80() {
    POAGraph graph;
    graph.numNodes = 80;
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
    graph.nodeLabels[20] = 'A';
    graph.nodeLabels[21] = 'F';
    graph.nodeLabels[22] = 'B';
    graph.nodeLabels[23] = 'C';
    graph.nodeLabels[24] = 'D';
    graph.nodeLabels[25] = 'E';
    graph.nodeLabels[26] = 'E';
    graph.nodeLabels[27] = 'E';
    graph.nodeLabels[28] = 'E';
    graph.nodeLabels[29] = 'E';
    graph.nodeLabels[30] = 'A';
    graph.nodeLabels[31] = 'F';
    graph.nodeLabels[32] = 'B';
    graph.nodeLabels[33] = 'C';
    graph.nodeLabels[34] = 'D';
    graph.nodeLabels[35] = 'E';
    graph.nodeLabels[36] = 'E';
    graph.nodeLabels[37] = 'E';
    graph.nodeLabels[38] = 'E';
    graph.nodeLabels[39] = 'E';
    graph.nodeLabels[40] = 'A';
    graph.nodeLabels[41] = 'F';
    graph.nodeLabels[42] = 'B';
    graph.nodeLabels[43] = 'C';
    graph.nodeLabels[44] = 'D';
    graph.nodeLabels[45] = 'E';
    graph.nodeLabels[46] = 'E';
    graph.nodeLabels[47] = 'E';
    graph.nodeLabels[48] = 'E';
    graph.nodeLabels[49] = 'E';
    graph.nodeLabels[50] = 'A';
    graph.nodeLabels[51] = 'F';
    graph.nodeLabels[52] = 'B';
    graph.nodeLabels[53] = 'C';
    graph.nodeLabels[54] = 'D';
    graph.nodeLabels[55] = 'E';
    graph.nodeLabels[56] = 'E';
    graph.nodeLabels[57] = 'E';
    graph.nodeLabels[58] = 'E';
    graph.nodeLabels[59] = 'E';
    graph.nodeLabels[60] = 'A';
    graph.nodeLabels[61] = 'F';
    graph.nodeLabels[62] = 'B';
    graph.nodeLabels[63] = 'C';
    graph.nodeLabels[64] = 'D';
    graph.nodeLabels[65] = 'E';
    graph.nodeLabels[66] = 'E';
    graph.nodeLabels[67] = 'E';
    graph.nodeLabels[68] = 'E';
    graph.nodeLabels[69] = 'E';
    graph.nodeLabels[70] = 'A';
    graph.nodeLabels[71] = 'F';
    graph.nodeLabels[72] = 'B';
    graph.nodeLabels[73] = 'C';
    graph.nodeLabels[74] = 'D';
    graph.nodeLabels[75] = 'E';
    graph.nodeLabels[76] = 'E';
    graph.nodeLabels[77] = 'E';
    graph.nodeLabels[78] = 'E';
    graph.nodeLabels[79] = 'E';
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
    graph.nodeNumEdges[20] = 1; graph.nodeIncomingEdges[0][20] = 19;
    graph.nodeNumEdges[21] = 1; graph.nodeIncomingEdges[0][21] = 20;
    graph.nodeNumEdges[22] = 1; graph.nodeIncomingEdges[0][22] = 21;
    graph.nodeNumEdges[23] = 1; graph.nodeIncomingEdges[0][23] = 22;
    graph.nodeNumEdges[24] = 1; graph.nodeIncomingEdges[0][24] = 23;
    graph.nodeNumEdges[25] = 1; graph.nodeIncomingEdges[0][25] = 24;
    graph.nodeNumEdges[26] = 1; graph.nodeIncomingEdges[0][26] = 25;
    graph.nodeNumEdges[27] = 1; graph.nodeIncomingEdges[0][27] = 26;
    graph.nodeNumEdges[28] = 1; graph.nodeIncomingEdges[0][28] = 27;
    graph.nodeNumEdges[29] = 1; graph.nodeIncomingEdges[0][29] = 28;
    graph.nodeNumEdges[30] = 1; graph.nodeIncomingEdges[0][30] = 29;
    graph.nodeNumEdges[31] = 1; graph.nodeIncomingEdges[0][31] = 30;
    graph.nodeNumEdges[32] = 1; graph.nodeIncomingEdges[0][32] = 31;
    graph.nodeNumEdges[33] = 1; graph.nodeIncomingEdges[0][33] = 32;
    graph.nodeNumEdges[34] = 1; graph.nodeIncomingEdges[0][34] = 33;
    graph.nodeNumEdges[35] = 1; graph.nodeIncomingEdges[0][35] = 34;
    graph.nodeNumEdges[36] = 1; graph.nodeIncomingEdges[0][36] = 35;
    graph.nodeNumEdges[37] = 1; graph.nodeIncomingEdges[0][37] = 36;
    graph.nodeNumEdges[38] = 1; graph.nodeIncomingEdges[0][38] = 37;
    graph.nodeNumEdges[39] = 1; graph.nodeIncomingEdges[0][39] = 38;
    graph.nodeNumEdges[40] = 1; graph.nodeIncomingEdges[0][40] = 39;
    graph.nodeNumEdges[41] = 1; graph.nodeIncomingEdges[0][41] = 40;
    graph.nodeNumEdges[42] = 1; graph.nodeIncomingEdges[0][42] = 41;
    graph.nodeNumEdges[43] = 1; graph.nodeIncomingEdges[0][43] = 42;
    graph.nodeNumEdges[44] = 1; graph.nodeIncomingEdges[0][44] = 43;
    graph.nodeNumEdges[45] = 1; graph.nodeIncomingEdges[0][45] = 44;
    graph.nodeNumEdges[46] = 1; graph.nodeIncomingEdges[0][46] = 45;
    graph.nodeNumEdges[47] = 1; graph.nodeIncomingEdges[0][47] = 46;
    graph.nodeNumEdges[48] = 1; graph.nodeIncomingEdges[0][48] = 47;
    graph.nodeNumEdges[49] = 1; graph.nodeIncomingEdges[0][49] = 48;
    graph.nodeNumEdges[50] = 1; graph.nodeIncomingEdges[0][50] = 49;
    graph.nodeNumEdges[51] = 1; graph.nodeIncomingEdges[0][51] = 50;
    graph.nodeNumEdges[52] = 1; graph.nodeIncomingEdges[0][52] = 51;
    graph.nodeNumEdges[53] = 1; graph.nodeIncomingEdges[0][53] = 52;
    graph.nodeNumEdges[54] = 1; graph.nodeIncomingEdges[0][54] = 53;
    graph.nodeNumEdges[55] = 1; graph.nodeIncomingEdges[0][55] = 54;
    graph.nodeNumEdges[56] = 1; graph.nodeIncomingEdges[0][56] = 55;
    graph.nodeNumEdges[57] = 1; graph.nodeIncomingEdges[0][57] = 56;
    graph.nodeNumEdges[58] = 1; graph.nodeIncomingEdges[0][58] = 57;
    graph.nodeNumEdges[59] = 1; graph.nodeIncomingEdges[0][59] = 58;
    graph.nodeNumEdges[60] = 1; graph.nodeIncomingEdges[0][60] = 59;
    graph.nodeNumEdges[61] = 1; graph.nodeIncomingEdges[0][61] = 60;
    graph.nodeNumEdges[62] = 1; graph.nodeIncomingEdges[0][62] = 61;
    graph.nodeNumEdges[63] = 1; graph.nodeIncomingEdges[0][63] = 62;
    graph.nodeNumEdges[64] = 1; graph.nodeIncomingEdges[0][64] = 63;
    graph.nodeNumEdges[65] = 1; graph.nodeIncomingEdges[0][65] = 64;
    graph.nodeNumEdges[66] = 1; graph.nodeIncomingEdges[0][66] = 65;
    graph.nodeNumEdges[67] = 1; graph.nodeIncomingEdges[0][67] = 66;
    graph.nodeNumEdges[68] = 1; graph.nodeIncomingEdges[0][68] = 67;
    graph.nodeNumEdges[69] = 1; graph.nodeIncomingEdges[0][69] = 68;
    graph.nodeNumEdges[70] = 1; graph.nodeIncomingEdges[0][70] = 69;
    graph.nodeNumEdges[71] = 1; graph.nodeIncomingEdges[0][71] = 70;
    graph.nodeNumEdges[72] = 1; graph.nodeIncomingEdges[0][72] = 71;
    graph.nodeNumEdges[73] = 1; graph.nodeIncomingEdges[0][73] = 72;
    graph.nodeNumEdges[74] = 1; graph.nodeIncomingEdges[0][74] = 73;
    graph.nodeNumEdges[75] = 1; graph.nodeIncomingEdges[0][75] = 74;
    graph.nodeNumEdges[76] = 1; graph.nodeIncomingEdges[0][76] = 75;
    graph.nodeNumEdges[77] = 1; graph.nodeIncomingEdges[0][77] = 76;
    graph.nodeNumEdges[78] = 1; graph.nodeIncomingEdges[0][78] = 77;
    graph.nodeNumEdges[79] = 1; graph.nodeIncomingEdges[0][79] = 78;


    // Perform topological sorting (modifies the graph directly)
    topologicalSort(graph);

    std::cout << "Topologically Sorted Nodes: ";
    for (int i = 0; i < graph.numNodes; i++) {
        std::cout << graph.nodeLabels[i] << " ";
    }
    std::cout << std::endl;

    char seq[MAX_SEQ_LENGTH+1] = "AFBCDEEEEEAFBCDEEEEEAFBCDEEEEEAFBCDEEEEEAFBCDEEEEEAFBCDEEEEEAFBCDEEEEEAFBCDEEEEE";
    int seqSize[1] = {80};
    char alignedSeq[MAX_SEQ_LENGTH+1] = {0};
    char alignedInputSeq[MAX_SEQ_LENGTH+1] = {0};
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
    test_POA_basic_sw_80();
    return 0;
}
