#include <iostream>
#include <vector>
#include <algorithm>  // for std::fill
#include <iomanip>    // for std::setw
#include <cstring>    // For memcpy
#include <string>     // For std::string
#include <fstream>    // For file operations
#include <sstream>    // For string stream
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"
#include "xrt/xrt_bo.h"
#include "../defines.hpp"
#include <chrono>
#include <cmath>
#include <numeric>    // For std::accumulate

static int numTiles(int value, int x) {
    return std::ceil((value + x - 1) / x);
}
static int ceilToMultiple(int value, int x) {
    return ((value + x - 1) / x) * x;
}

struct Sequence {
    std::string id;
    std::string description;
    std::string sequence;
};

// Static variable to hold sequence IDs for output file
static std::string seqIds[2] = {"", ""};

// Helper function to truncate and display strings
std::string truncateString(const std::string& str, int maxLength = 30) {
    if (str.length() <= maxLength) {
        return str;
    }
    return str.substr(0, maxLength) + "... [" + std::to_string(str.length()) + " characters long - not displayed]";
}

// Function to load sequences from FASTA-like format file
std::vector<Sequence> loadSequences(const std::string& filename) {
    std::vector<Sequence> sequences;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return sequences;
    }
    
    std::string line;
    Sequence currentSeq;
    bool inSequence = false;
    
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        
        // Check if line is a header (starts with '>')
        if (line[0] == '>') {
            // If we were already processing a sequence, save it
            if (inSequence && !currentSeq.sequence.empty()) {
                sequences.push_back(currentSeq);
            }
            
            // Start a new sequence
            currentSeq = Sequence();
            inSequence = true;
            
            // Parse the header
            // Format: >ID [metadata]
            std::string header = line.substr(1); // Remove the '>' character
            size_t firstSpace = header.find(' ');
            
            if (firstSpace != std::string::npos) {
                currentSeq.id = header.substr(0, firstSpace);
                currentSeq.description = header.substr(firstSpace + 1);
            } else {
                currentSeq.id = header;
                currentSeq.description = "";
            }
            
            currentSeq.sequence = "";
        } else if (inSequence) {
            // Add this line to the current sequence
            // Remove any whitespace characters
            line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
            currentSeq.sequence += line;
        }
    }
    
    // Don't forget to save the last sequence
    if (inSequence && !currentSeq.sequence.empty()) {
        sequences.push_back(currentSeq);
    }
    
    file.close();
    return sequences;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -f <filename>       Specify input file with sequences (FASTA format)" << std::endl;
    std::cout << "  -x <directory>      Specify directory where build_dir.hw.xilinx_u250_gen3x16_xdma_4_1_202210_1/SW_syst.link.xclbin is located" << std::endl;
    std::cout << "  -o [filename]       Output the full alignment results to a text file" << std::endl;
    std::cout << "                      (default: alignment_results.txt if no filename provided)" << std::endl;
    std::cout << "  -s <tile_size>      Specify the tile dimension size (mandatory)" << std::endl;
    std::cout << "  -h                  Display this help message" << std::endl;
}

// Function to run a single execution and return the time
double runSingleExecution(const std::string& seq1, const std::string& seq2, 
                         xrt::device& myDevice, xrt::xclbin& xclbin, 
                         int tileDimension, bool printOutput = true,
                         const std::string& outputFile = "") {
    // Define input sequences
    int seqsize[2] = {static_cast<int>(seq1.length()), 
                      static_cast<int>(seq2.length())}; // Original sequence lengths
    
    // Calculate padded sizes and number of tiles
    int inputsize1 = ceilToMultiple(seqsize[0], tileDimension) + 1;
    int inputsize2 = ceilToMultiple(seqsize[1], tileDimension) + 1;
    int numTilesVar[2] = {
        numTiles(seqsize[0], tileDimension), // Number of horizontal tiles
        numTiles(seqsize[1], tileDimension)  // Number of vertical tiles
    };
    
    if (printOutput) {
        std::cout << "Input sizes: " << inputsize1 << " || " << inputsize2 << std::endl;
        std::cout << "Tile counts: " << numTilesVar[0] << " || " << numTilesVar[1] << std::endl;
    }
    
    // Allocate memory for sequences
    char* seq1Buf = (char*)malloc(inputsize1 * sizeof(char));
    char* seq2Buf = (char*)malloc(inputsize2 * sizeof(char));
    
    // Copy sequences
    strcpy(seq1Buf, seq1.c_str());
    strcpy(seq2Buf, seq2.c_str());
    
    // Output buffers for aligned sequences
    char* alignedSeq1 = (char*)malloc((seqsize[0]+seqsize[1]) * sizeof(char));
    char* alignedSeq2 = (char*)malloc((seqsize[0]+seqsize[1]) * sizeof(char));
    
    // Pointer for score buffer - size needs to include the boundary cells
    int buffer_horz_size = numTilesVar[0] * (tileDimension + 1)  * 2;
    int buffer_vert_size = numTilesVar[1];

    // Allocate buffer
    int* buffer = (int*)malloc(sizeof(int) * buffer_horz_size * buffer_vert_size);

    // Print input with truncation
    if (printOutput) {
        std::cout << "Sequence 1: " << truncateString(seq1Buf) << std::endl;
        std::cout << "Sequence 2: " << truncateString(seq2Buf) << std::endl;
    }

    #ifdef DEBUG
        if (printOutput) std::cout << "Input Load Complete" << std::endl;
    #endif

    auto uuid = myDevice.load_xclbin(xclbin);

    xrt::kernel SW_basic_linear(myDevice, uuid, "SW_basic_linear");

    xrt::run RunObj = xrt::run(SW_basic_linear);

    // Make the buffers
    auto seq1_bo = xrt::bo(myDevice, sizeof(char) * inputsize1, 
                           SW_basic_linear.group_id(0));
    auto seq2_bo = xrt::bo(myDevice, sizeof(char) * inputsize2, 
                           SW_basic_linear.group_id(1));
    auto buffer_bo = xrt::bo(myDevice, sizeof(int) * buffer_horz_size * buffer_vert_size, 
                            xrt::bo::flags::device_only, SW_basic_linear.group_id(2));
    auto seqsz_bo = xrt::bo(myDevice, sizeof(int) * 2, 
                            SW_basic_linear.group_id(5));
    auto tilenum_bo = xrt::bo(myDevice, sizeof(int) * 2, 
                            SW_basic_linear.group_id(6));
    auto align1_bo = xrt::bo(myDevice, sizeof(char) * (seqsize[0]+seqsize[1]), 
                            SW_basic_linear.group_id(3));
    auto align2_bo = xrt::bo(myDevice, sizeof(char) * (seqsize[0]+seqsize[1]), 
                            SW_basic_linear.group_id(4));

    #ifdef DEBUG
        if (printOutput) std::cout << "Buffers Created " << std::endl;
    #endif

    // Copy host data
    auto start = std::chrono::high_resolution_clock::now();
    seq1_bo.write(seq1Buf, sizeof(char) * inputsize1, 0);
    seq2_bo.write(seq2Buf, sizeof(char) * inputsize2, 0);
    seqsz_bo.write(seqsize, sizeof(int) * 2, 0);
    tilenum_bo.write(numTilesVar, sizeof(int) * 2, 0);
    // DO NOT CALL WRITE OR SYNC FOR BUFFER
    seq1_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    seq2_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    seqsz_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    tilenum_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    #ifdef DEBUG
        if (printOutput) std::cout << "Buffers Written " << std::endl;
    #endif

    // Run the kernel
    RunObj.set_arg(0, seq1_bo);
    RunObj.set_arg(1, seq2_bo);
    RunObj.set_arg(2, buffer_bo);
    RunObj.set_arg(3, seqsz_bo);
    RunObj.set_arg(4, tilenum_bo);
    RunObj.set_arg(5, align1_bo);
    RunObj.set_arg(6, align2_bo);
    RunObj.start();
    RunObj.wait();

    #ifdef DEBUG
        if (printOutput) std::cout << "Execution Complete " << std::endl;
    #endif    

    // Read back the output
    align1_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    align2_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    
    #ifdef DEBUG
        if (printOutput) std::cout << "Execution Complete " << std::endl;
    #endif  
    
    align1_bo.read(alignedSeq1);
    align2_bo.read(alignedSeq2);

    // Reverse the aligned sequences
    std::reverse(alignedSeq1, alignedSeq1 + strlen(alignedSeq1));
    std::reverse(alignedSeq2, alignedSeq2 + strlen(alignedSeq2));

    if (printOutput) {
        // Print results with truncation
        std::cout << "Aligned 1 : " << truncateString(alignedSeq1) << std::endl;
        std::cout << "Aligned 2 : " << truncateString(alignedSeq2) << std::endl;
    }
    
    // Output to file if specified
    if (!outputFile.empty()) {
        std::ofstream outFile(outputFile);
        if (outFile.is_open()) {
            // Calculate the full alignment length
            size_t alignLen = strlen(alignedSeq1);
            
            // Write header
            outFile << "Smith-Waterman Alignment Results" << std::endl;
            outFile << "===============================" << std::endl << std::endl;
            outFile << "Alignment length: " << alignLen << " bases" << std::endl << std::endl;
            
            // Add sequence IDs if available (passed through a static variable)
            if (!seqIds[0].empty() && !seqIds[1].empty()) {
                outFile << "Sequence 1 ID: " << seqIds[0] << std::endl;
                outFile << "Sequence 2 ID: " << seqIds[1] << std::endl << std::endl;
            }
            
            // Write the full sequences without truncation
            outFile << "Sequence 1: " << std::endl;
            outFile << alignedSeq1 << std::endl << std::endl;
            
            outFile << "Sequence 2: " << std::endl;
            outFile << alignedSeq2 << std::endl << std::endl;
            
            // Display alignment with match indicators
            outFile << "Match visualization:" << std::endl;
            
            // Define chunk size for better readability
            const int CHUNK_SIZE = 80;
            for (size_t i = 0; i < alignLen; i += CHUNK_SIZE) {
                size_t endPos = std::min(i + CHUNK_SIZE, alignLen);
                
                // Write a chunk of the first sequence
                outFile << "Seq1: ";
                for (size_t j = i; j < endPos; j++) {
                    outFile << alignedSeq1[j];
                }
                outFile << std::endl;
                
                // Write match/mismatch indicators
                outFile << "      ";
                for (size_t j = i; j < endPos; j++) {
                    // Check if there's a gap in either sequence
                    if (alignedSeq1[j] == '-' || alignedSeq2[j] == '-') {
                        outFile << ' ';
                    } 
                    // Check for exact match
                    else if (alignedSeq1[j] == alignedSeq2[j]) {
                        outFile << '|';
                    } 
                    // Mismatch
                    else {
                        outFile << '.';
                    }
                }
                outFile << std::endl;
                
                // Write a chunk of the second sequence
                outFile << "Seq2: ";
                for (size_t j = i; j < endPos; j++) {
                    outFile << alignedSeq2[j];
                }
                outFile << std::endl << std::endl;
            }
            
            outFile.close();
            std::cout << "Alignment results written to " << outputFile << std::endl;
        } else {
            std::cerr << "Error: Could not open output file " << outputFile << std::endl;
        }
    }

    // Free memory
    std::free(seq1Buf);
    std::free(seq2Buf);
    std::free(alignedSeq1);
    std::free(alignedSeq2);
    std::free(buffer);

    return duration.count(); // Return execution time
}

int main(int argc, char* argv[]) {
    std::string testFilePath = ""; // Must be set by -f flag
    std::string xclbinDir = "."; // Default to current directory
    std::string outputFile = ""; // Optional output file
    int tileDimension = -1; // Must be set by -s flag
    
    // FPGA-only implementation
    xrt::device myDevice; 
    xrt::xclbin xclbin;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-f" && i + 1 < argc) {
            testFilePath = argv[i + 1]; // Get the file path
            i++; // Skip the next argument since we've processed it
        } else if (std::string(argv[i]) == "-x" && i + 1 < argc) {
            xclbinDir = argv[i + 1]; // Get the directory path
            i++; // Skip the next argument since we've processed it
        } else if (std::string(argv[i]) == "-o") {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                // If there's an argument after -o and it doesn't start with a dash (not another flag)
                outputFile = argv[i + 1]; // Get the output file path
                i++; // Skip the next argument since we've processed it
            } else {
                // If -o is specified but no filename is given, use default name
                outputFile = "alignment_results.txt";
            }
        } else if (std::string(argv[i]) == "-s" && i + 1 < argc) {
            tileDimension = std::atoi(argv[i + 1]); // Get the tile dimension
            if (tileDimension <= 0) {
                std::cerr << "Error: Tile dimension must be positive." << std::endl;
                return 1;
            }
            i++; // Skip the next argument since we've processed it
        } else if (std::string(argv[i]) == "-h") {
            printUsage(argv[0]);
            return 0;
        }
    }
    
    // Check if mandatory parameters were specified
    if (testFilePath.empty()) {
        std::cerr << "Error: Input file (-f) is mandatory." << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    if (tileDimension == -1) {
        std::cerr << "Error: Tile dimension (-s) is mandatory." << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    std::cout << "Using tile dimension: " << tileDimension << std::endl;
    
    // Construct the full xclbin path by appending the required path to the specified directory
    std::string fullXclbinPath;
    // Add trailing slash to directory if not present
    if (!xclbinDir.empty() && xclbinDir.back() != '/') {
        xclbinDir += '/';
    }
    fullXclbinPath = xclbinDir + "build_dir.hw.xilinx_u250_gen3x16_xdma_4_1_202210_1/SW_syst.link.xclbin";
    
    // Always use FPGA mode
    std::cout << "Running in FPGA mode" << std::endl;
    std::cout << "Using xclbin file: " << fullXclbinPath << std::endl;
    xclbin = xrt::xclbin(fullXclbinPath);
    myDevice = xrt::device("0000:88:00.1");

    // Load sequences from file
    std::cout << "Using input file: " << testFilePath << std::endl;
    std::vector<Sequence> sequences = loadSequences(testFilePath);
    
    if (sequences.empty()) {
        std::cerr << "No sequences found in file '" << testFilePath << "'." << std::endl;
        return 1;
    }
    
    // Display all sequences and let user select
    std::cout << "Found " << sequences.size() << " sequences in the file:" << std::endl;
    for (size_t i = 0; i < sequences.size(); i++) {
        std::cout << i << ": " << sequences[i].id 
                  << " (" << sequences[i].sequence.length() << " bases)" 
                  << (sequences[i].description.empty() ? "" : " - " + truncateString(sequences[i].description))
                  << std::endl;
    }
    
    // Get user selection for first sequence
    int seq1Index = -1;
    while (seq1Index < 0 || seq1Index >= static_cast<int>(sequences.size())) {
        std::cout << "Select first sequence (0-" << (sequences.size() - 1) << "): ";
        std::cin >> seq1Index;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input. Please enter a number." << std::endl;
            seq1Index = -1;
        }
    }
    
    // Get user selection for second sequence
    int seq2Index = -1;
    while (seq2Index < 0 || seq2Index >= static_cast<int>(sequences.size()) || seq2Index == seq1Index) {
        std::cout << "Select second sequence (0-" << (sequences.size() - 1) << ", different from first): ";
        std::cin >> seq2Index;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input. Please enter a number." << std::endl;
            seq2Index = -1;
        } else if (seq2Index == seq1Index) {
            std::cout << "Please select a different sequence from the first one." << std::endl;
        }
    }
    
    // Get the selected sequences
    std::string seq1 = sequences[seq1Index].sequence;
    std::string seq2 = sequences[seq2Index].sequence;
    
    std::cout << "Selected sequences:" << std::endl;
    std::cout << "Sequence 1: " << sequences[seq1Index].id << " (" << seq1.length() << " bases)" << std::endl;
    std::cout << "Sequence 2: " << sequences[seq2Index].id << " (" << seq2.length() << " bases)" << std::endl;
    
    // Run the alignment
    // Store sequence IDs for output file
    seqIds[0] = sequences[seq1Index].id;
    seqIds[1] = sequences[seq2Index].id;
    
    double time = runSingleExecution(seq1, seq2, myDevice, xclbin, tileDimension, true, outputFile);
    std::cout << "Total time taken: " << time << " seconds" << std::endl;
    
    std::cout << "Alignment completed." << std::endl;
    return 0; // Success
}