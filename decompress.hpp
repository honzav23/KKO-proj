#ifndef DECOMPRESS_HPP
#define DECOMPRESS_HPP

#include "common.hpp"

/**
 * @brief Decodes canonical Huffman code
 * 
 * @param numOfDifferentSymbols Number of distinct symbols in a file
 * @param fp File 
 * @return Unordered map (code, symbol)
 */
unordered_map<string, Pixel> decodeCanonicalHuffman(int numOfDifferentSymbols, FILE *fp);

/**
 * @brief Extracts block information from a file
 * 
 * @param fp File descriptor
 * @param numOfBlocks Total number of blocks
 * @return Vector of information about blocks
 */
vector<BlockInfo> getBlockInfoFromFile(FILE *fp, int numOfBlocks);

/**
 * @brief Gets image symbols from a file
 * 
 * @param canonicalHuffmanSwap Swapped Huffman code, unordered map (code, symbol)
 * @param fileContent File data
 * @param args Command line arguments
 * @return Vector of symbols
 */
vector<Pixel> getImageData(unordered_map<string, Pixel> &canonicalHuffmanSwap,
                    vector<u_char> &fileContent, CommandArgs &args);

/**
 * @brief Assigns data to original block buffer for one block only
 * 
 * @param blockInfoDecompress Information about block
 * @param canonicalHuffmanSwap Swapped Huffman code, unordered map (code, symbol)
 * @param args Command line arguments
 * @param fileContent File data
 */
void getDataForOneBlock(vector<BlockInfo> &blockInfoDecompress, unordered_map<string, Pixel> &canonicalHuffmanSwap, 
            CommandArgs &args, vector<u_char> &fileContent);

/**
 * @brief Assigns data to original block buffer for one multiple blocks
 * 
 * @param blockInfoDecompress Information about block
 * @param canonicalHuffmanSwap Swapped Huffman code, unordered map (code, symbol)
 * @param args Command line arguments
 * @param fileContent File data
 */
void getDataForMultipleBlocks(vector<BlockInfo> &blockInfoDecompress, unordered_map<string, Pixel> &canonicalHuffmanSwap, 
            CommandArgs &args, vector<u_char> &fileContent);

/**
 * @brief Writes decompressed data to output file
 * 
 * @param blockInfoDecompress Information about the blocks
 * @param args Command line arguments
 */
void writeDecompressedToFile(vector<BlockInfo> &blockInfoDecompress, CommandArgs &args);

/**
 * @brief Decompresses the input file
 * 
 * @param args Command line arguments
 */
void decompressFile(CommandArgs &args);

#endif