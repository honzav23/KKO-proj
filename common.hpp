#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdint>
#include <unistd.h>
#include <cstring>
#include <tuple>
#include <bitset>
#include <unordered_map>

using namespace std;

typedef int16_t Pixel;

#define BLOCK_SIZE 16

enum compressMode {COMPRESS, DECOMPRESS};

typedef struct commandArgs {
    // If true compress image, if false decompress
    enum compressMode mode = COMPRESS;
    bool model = false;
    bool adaptiveScan = false;
    string inputFile;
    string outputFile;
    int width;
    bool help = false;

} CommandArgs;

typedef struct blockInfo {
    bool compressed = true;
    bool horizontal = true;
    bool model = false;

    // (number of repetitions, symbol)
    vector<tuple<uint8_t, Pixel>> rleForBestOption = {};
    vector<vector<Pixel>> originalBlock = {};

} BlockInfo;

/**
 * @brief Parses arguments from command line
 * 
 * @param argc Number of arguments
 * @param argv Arguments
 * @return Struct containing all the arguments
 */
CommandArgs parseArguments(int argc, char *argv[]);

/**
 * @brief Prints help
 * 
 */
void printHelp();

/**
 * @brief Creates canonical Huffman code from code lengths
 * 
 * @param symbolsWithCodeLengths Vector of tuples (symbol, code length)
 * @return Unordered map (symbol, code) 
 */
unordered_map<Pixel, string> getCodeFromLengths(vector<tuple<Pixel, int>> &symbolsWithCodeLengths);

#endif