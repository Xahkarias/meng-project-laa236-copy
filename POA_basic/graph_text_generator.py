import argparse  # Ensure this import is included
def generate_graph(X):
    # Create the graph block text
    graph_block = f"graph.numNodes = {X};\n"
    
    # Create nodeLabels list
    for i in range(X):
        if i % 10 == 0:
            graph_block += f"    graph.nodeLabels[{i}] = 'A';\n"
        elif i % 10 == 1:
            graph_block += f"    graph.nodeLabels[{i}] = 'F';\n"
        elif i % 10 == 2:
            graph_block += f"    graph.nodeLabels[{i}] = 'B';\n"
        elif i % 10 == 3:
            graph_block += f"    graph.nodeLabels[{i}] = 'C';\n"
        elif i % 10 == 4:
            graph_block += f"    graph.nodeLabels[{i}] = 'D';\n"
        else:
            graph_block += f"    graph.nodeLabels[{i}] = 'E';\n"
    
    # Create nodeNumEdges and nodeIncomingEdges
    for i in range(X):
        if i == 0:
            graph_block += f"    graph.nodeNumEdges[{i}] = 0;\n"
        else:
            graph_block += f"    graph.nodeNumEdges[{i}] = 1; graph.nodeIncomingEdges[0][{i}] = {i-1};\n"
    
    return graph_block

def save_graph_to_file(X, filename):
    graph_text = generate_graph(X)
    
    # Open the file and write the graph text to it
    with open(filename, 'w') as file:
        file.write(graph_text)

def main():
    # Setup argument parsing for command-line input
    parser = argparse.ArgumentParser(description="Generate graph block with given number of nodes")
    parser.add_argument("X", type=int, help="Number of nodes in the graph")
    #parser.add_argument("filename", type=str, help="Output file to store the graph block")
    
    args = parser.parse_args()

    # Get the number of nodes (X) and the filename from command-line arguments
    X = args.X
    #filename = args.filename

    # Save the graph block to the file
    save_graph_to_file(X, "graph_output.txt")
    
    print(f"Graph data saved to graph_output.txt")

if __name__ == "__main__":
    main()