#include "decompress.hpp"

unordered_map<string, Pixel> decodeCanonicalHuffman(int numOfDifferentSymbols, FILE *fp) {
    vector<tuple<Pixel, int>> symbolsWithCodeLen = {};

    // Reading 16 bit numbers
    u_char bytes[2] = {};
    Pixel symbol;
    int codeLen;
    for (int i = 0; i < numOfDifferentSymbols; i++) {
        fread(bytes, 1, 2, fp);
        symbol = (bytes[0] << 8) | bytes[1];
        fread(bytes, 1, 2, fp);
        codeLen = (bytes[0] << 8) | bytes[1];
        tuple<Pixel, int> p = make_pair(symbol, codeLen);
        symbolsWithCodeLen.push_back(p);
    }

    unordered_map<Pixel, string> canonicalHuffman = getCodeFromLengths(symbolsWithCodeLen);
    unordered_map<string, Pixel> canonicalHufSwap;
    for (auto elem : canonicalHuffman) {
        canonicalHufSwap[elem.second] = elem.first;
    }

    return canonicalHufSwap;
}

vector<BlockInfo> getBlockInfoFromFile(FILE *fp, int numOfBlocks) {
    vector<BlockInfo> info = {};
    int bitsRead = 0;
    u_char currentByte = 0;
    for (int i = 0; i < numOfBlocks; i++) {
        BlockInfo block;
        if (bitsRead % 8 == 0) {
            currentByte = fgetc(fp);
            bitsRead = 0;
        }
        block.compressed = (currentByte >> (7 - bitsRead)) % 2;
        bitsRead++;
        if (bitsRead % 8 == 0) {
            currentByte = fgetc(fp);
            bitsRead = 0;
        }
        block.horizontal = (currentByte >> (7 - bitsRead)) % 2;
        bitsRead++;
        if (bitsRead % 8 == 0) {
            currentByte = fgetc(fp);
            bitsRead = 0;
        }
        block.model = (currentByte >> (7 - bitsRead)) % 2;
        bitsRead++;

        info.push_back(block);
    }
    return info;
}

vector<Pixel> getImageData(unordered_map<string, Pixel> &canonicalHuffmanSwap,
                    vector<u_char> &fileContent, CommandArgs &args) {
    
    Pixel symbolToWrite = 0;
    string current = "";
    vector<Pixel> block = {};
    for (auto symb : fileContent) {
        for (int i = 0; i < 8; i++) {
            current += (symb >> (7 - i)) % 2 == 0 ? "0" : "1";

            if (canonicalHuffmanSwap.find(current) != canonicalHuffmanSwap.end()) {
                symbolToWrite = canonicalHuffmanSwap[current];
                if (symbolToWrite == 256) {
                    break;
                }
                block.push_back(symbolToWrite);
                current = "";
            }
        }
        if (symbolToWrite == 256) {
            break;
        }
    }
    return block;
}

void getDataForOneBlock(vector<BlockInfo> &blockInfoDecompress, unordered_map<string, Pixel> &canonicalHuffmanSwap, 
            CommandArgs &args, vector<u_char> &fileContent) {

    vector<Pixel> imageData = getImageData(canonicalHuffmanSwap, fileContent, args);

    // Unwrap RLE format to normal format
    if (blockInfoDecompress[0].compressed) {
        vector<Pixel> newBlock = {};
        for (size_t i = 0; i < imageData.size(); i++) {
            if (imageData[i] != RLE_MARKER) {
                newBlock.push_back(imageData[i]);
            }
            else {
                for (int j = 0; j < imageData[i+1]; j++) {
                    newBlock.push_back(imageData[i+2]);
                }
                i += 2;
            }
        }
        imageData = newBlock;
    }

    // Create normal pixels from pixel differences
    if (blockInfoDecompress[0].model) {
        for (size_t i = 1; i < imageData.size(); i++) {
            imageData[i] += imageData[i-1];
        }
    }
    // Put in 2D originalBlock
    vector<Pixel> row = {};
    for (auto b : imageData) {
        row.push_back(b);
        if ((int) row.size() == args.width) {
            blockInfoDecompress[0].originalBlock.push_back(row);
            row = {};
        }
    }
}

void getDataForMultipleBlocks(vector<BlockInfo> &blockInfoDecompress, unordered_map<string, Pixel> &canonicalHuffmanSwap, 
            CommandArgs &args, vector<u_char> &fileContent) {

    vector<Pixel> imageData = getImageData(canonicalHuffmanSwap, fileContent, args);

    int symbolCounter = 0;
    for (size_t i = 0; i < blockInfoDecompress.size(); i++) {
        int s;
        vector<Pixel> modifiedImageData = {};

        int rleCounter = 0;

        // If RLE is used
        if (blockInfoDecompress[i].compressed) {
            while (rleCounter < 256) {
                if (imageData[symbolCounter] != RLE_MARKER) {
                    modifiedImageData.push_back(imageData[symbolCounter++]);
                    rleCounter++;
                }
                else {
                    symbolCounter++;
                    int reps = imageData[symbolCounter++];
                    for (int j = 0; j < reps; j++) {
                        modifiedImageData.push_back(imageData[symbolCounter]);
                    }
                    symbolCounter++;
                    rleCounter += reps;
                }
            }
        }
        else {
            s = symbolCounter;
            for (int j = s; j < s + 256; j++) {
                modifiedImageData.push_back(imageData[j]);
                symbolCounter++;
            }
        }

        // Handle difference of pixels model
        if (blockInfoDecompress[i].model) {
            for (size_t j = 1; j < modifiedImageData.size(); j++) {
                modifiedImageData[j] += modifiedImageData[j-1];
            }
        }


        if (blockInfoDecompress[i].horizontal) {
            vector<Pixel> newBlock = {};
            for (auto im : modifiedImageData) {
                newBlock.push_back(im);
                if (newBlock.size() == BLOCK_SIZE) {
                    blockInfoDecompress[i].originalBlock.push_back(newBlock);
                    newBlock = {};
                }
            }
        }
        else {
            blockInfoDecompress[i].originalBlock.reserve(BLOCK_SIZE);
            for (int j = 0; j < BLOCK_SIZE; j++) {
                blockInfoDecompress[i].originalBlock.push_back({});
            }
            for (int j = 0; j < BLOCK_SIZE; j++) {
                for (int k = 0; k < BLOCK_SIZE; k++) {
                    int imageIndex = j * BLOCK_SIZE + k;
                    blockInfoDecompress[i].originalBlock[k].push_back(modifiedImageData[imageIndex]);
                }
            }
        }
    }
}

void writeDecompressedToFile(vector<BlockInfo> &blockInfoDecompress, CommandArgs &args) {
    FILE *fp = fopen(args.outputFile.c_str(), "wb");
    if (fp == nullptr) {
        cerr << "Unable to open file\n";
        exit(1);
    }
    if (blockInfoDecompress.size() == 1) {
        for (auto row : blockInfoDecompress[0].originalBlock) {
            for (auto col : row) {
                fputc((u_char)col, fp);
            }
        }
    }
    else {
        vector<vector<BlockInfo>> blockGrid = {};
        vector<BlockInfo> temp = {};

        for (auto b : blockInfoDecompress) {
            temp.push_back(b);
            if ((int) temp.size() == args.width / BLOCK_SIZE) {
                blockGrid.push_back(temp);
                temp = {};
            }
        }

        for (auto blockGridRow : blockGrid) {
            for (int i = 0; i < BLOCK_SIZE; i++) {
                int blockIndex = 0;
                for (int j = 0; j < args.width; j++) {
                    if (j % BLOCK_SIZE == 0 && j != 0) {
                        blockIndex++;
                    }
                    u_char symbolToWrite = blockGridRow[blockIndex].originalBlock[i][j % BLOCK_SIZE];
                    fputc(symbolToWrite, fp);
                }
            }
        }
    }
}

void decompressFile(CommandArgs &args) {
    FILE *fp = fopen(args.inputFile.c_str(), "rb");
    if (fp == nullptr) {
        cerr << "Unable to open file\n";
        exit(1);
    }
    u_char bytes[4] = {};
    int32_t numOfBlocks, numOfDifferentSymbols;
    fread(bytes, 1, 4, fp);
    args.width = (bytes[0] << 24) | (bytes[1] << BLOCK_SIZE) | (bytes[2] << 8) | bytes[3];
    fread(bytes, 1, 4, fp);
    numOfDifferentSymbols = (bytes[0] << 24) | (bytes[1] << BLOCK_SIZE) | (bytes[2] << 8) | bytes[3];
    fread(bytes, 1, 4, fp);
    numOfBlocks = (bytes[0] << 24) | (bytes[1] << BLOCK_SIZE) | (bytes[2] << 8) | bytes[3];

    unordered_map<string, Pixel> huffmanSwap = decodeCanonicalHuffman(numOfDifferentSymbols, fp);

    vector<BlockInfo> blockInfoDecompress = getBlockInfoFromFile(fp, numOfBlocks);

    vector<u_char> fileContent = {};

    int c;
    // Read the rest of the file (image data)
    while ((c = fgetc(fp)) != EOF) {
        fileContent.push_back((u_char) c);
    }

    if (blockInfoDecompress.size() == 1) {
        getDataForOneBlock(blockInfoDecompress, huffmanSwap, args, fileContent);
    }
    else {
        getDataForMultipleBlocks(blockInfoDecompress, huffmanSwap, args, fileContent);
    }
    fclose(fp);
    writeDecompressedToFile(blockInfoDecompress, args);
}