#ifndef COMPRESS_HPP
#define COMPRESS_HPP

#include "common.hpp"

/**
 * @brief Makes RLE from the grid
 * 
 * @param grid 2D grid containing data
 * @param horizontal If traversal through grid is done horizontally 
 * @return Vector of tuples (number of repetitions, symbol)
 */
vector<tuple<uint8_t, Pixel>> rleEncode(vector<vector<Pixel>> &grid, bool horizontal);

/**
 * @brief Splits the grid into 16x16 blocks
 * 
 * @param grid 2D Grid of data
 * @param args Command line arguments
 * @return Vector of 16x16 blocks 
 */
vector<vector<vector<Pixel>>> splitIntoBlocks(vector<vector<Pixel>> &grid, CommandArgs &args);

/**
 * @brief Applies difference of pixels model to a block
 * 
 * @param block Block to modify
 * @param horizontal If true, model is applied horizontally, else vertically
 * @return Modified block 
 */
vector<vector<Pixel>> applyModel(vector<vector<Pixel>> &block, bool horizontal);

/**
 * @brief Compares the compression based on their size
 * 
 * @param originalBlock Original block, nothing applied
 * @param horizontalRleNoModel Horizontal RLE applied, no model
 * @param horizontalRleWithModel Horizontal RLE applied, model applied
 * @param verticalRleNoModel Vertical RLE applied, no model
 * @param verticalRleWithModel Vertical RLE applied, model applied
 * @return Number from 1 to 5 depending on what is best
 * 1 - No compression
 * 2 - Horizontal RLE with no model the best
 * 3 - Horizontal RLE with model the best
 * 4 - Vertical RLE with no model the best
 * 5 - Vertical RLE with model the best
 */
int compareCompressions(vector<vector<Pixel>> &originalBlock, vector<tuple<uint8_t, Pixel>> &horizontalRleNoModel, vector<tuple<uint8_t, Pixel>> &horizontalRleWithModel,
                        vector<tuple<uint8_t, Pixel>> &verticalRleNoModel, vector<tuple<uint8_t, Pixel>> &verticalRleWithModel);

/**
 * @brief Creates 2D grid from the input file
 * 
 * @param args Command line arguments
 * @return 2D grid
 */
vector<vector<Pixel>> getGrid(CommandArgs &args);

/**
 * @brief Counts number of occurences for each symbol
 * 
 * @param info Information for each block 
 * @return Unordered map (symbol, number of occurences) 
 */
unordered_map<Pixel, int> getSymbolFrequencies(vector<BlockInfo> &info);

/**
 * @brief For each symbol counts the code length
 * 
 * @param symbolFrequencies Unordered map (symbol, number of occurences)  
 * @return Vector of tuples (symbol, code length)
 */
vector<tuple<Pixel, int>> getCodeLengths(unordered_map<Pixel, int> &symbolFrequencies);

/**
 * @brief Compresses the input file
 * 
 * @param args Command line arguments
 */
void compressFile(CommandArgs &args);

#endif