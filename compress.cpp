#include "compress.hpp"

vector<tuple<uint8_t, Pixel>> rleEncode(vector<vector<Pixel>> &grid, bool horizontal) {
    vector<tuple<uint8_t, Pixel>> rleWrite = {};
    int characterCnt = 1;
    Pixel previousPixel = grid[0][0];

    if (horizontal) {
        // Traverse from left to right
        for (size_t i = 0; i < grid.size(); i++) {
            for (size_t j = 0; j < grid[i].size(); j++) {
                if (i == 0 && j == 0) {
                    continue;
                }
                if (grid[i][j] == previousPixel && characterCnt < 255) {
                    characterCnt++;
                }
                else {
                    tuple<uint8_t, Pixel> tup = make_pair(characterCnt, previousPixel);
                    rleWrite.push_back(tup);
                    previousPixel = grid[i][j];
                    characterCnt = 1;
                }
            }
        }
    }
    else {
        // Traverse from up to bottom
        for (size_t i = 0; i < grid[0].size(); i++) {
            for (size_t j = 0; j < grid.size(); j++) {
                if (i == 0 && j == 0) {
                    continue;
                }
                if (grid[j][i] == previousPixel && characterCnt < 255) {
                    characterCnt++;
                }
                else {
                    tuple<uint8_t, Pixel> tup = make_pair(characterCnt, previousPixel);
                    rleWrite.push_back(tup);
                    previousPixel = grid[j][i];
                    characterCnt = 1;
                }
            }
        }
    }
    tuple<uint8_t, Pixel> tup = make_pair(characterCnt, previousPixel);
    rleWrite.push_back(tup);

    return rleWrite;

}

int compareCompressions(vector<vector<Pixel>> &originalBlock, vector<tuple<uint8_t, Pixel>> &horizontalRleNoModel, vector<tuple<uint8_t, Pixel>> &horizontalRleWithModel,
                        vector<tuple<uint8_t, Pixel>> &verticalRleNoModel, vector<tuple<uint8_t, Pixel>> &verticalRleWithModel) { 

    int originalModelSize = originalBlock.size() * originalBlock[0].size();
    int horizontalRleNoModelSize = horizontalRleNoModel.size() * 2;
    int horizontalRleWithModelSize = horizontalRleWithModel.size() * 2;
    int verticalRleNoModelSize = verticalRleNoModel.size() * 2;
    int verticalRleWithModelSize = verticalRleWithModel.size() * 2;

    int sizes[5] = {originalModelSize, horizontalRleNoModelSize, horizontalRleWithModelSize, verticalRleNoModelSize, verticalRleWithModelSize};
    int index = 0;
    int min = sizes[0];
    for (int i = 0; i < 5; i++) {
        if (sizes[i] < min && sizes[i] != 0) {
            min = sizes[i];
            index = i;
        }
    }
    return index + 1;
}

vector<BlockInfo> findBestCompression(vector<vector<vector<Pixel>>> &blocks, CommandArgs &args) {
    vector<BlockInfo> infos = {};
    for (auto b : blocks) {
        BlockInfo info;
        info.originalBlock = b;

        vector<tuple<uint8_t, Pixel>> horizontalRleNoModel = {};
        vector<tuple<uint8_t, Pixel>> horizontalRleWithModel = {};
        vector<tuple<uint8_t, Pixel>> verticalRleNoModel = {};
        vector<tuple<uint8_t, Pixel>> verticalRleWithModel = {};
        horizontalRleNoModel = rleEncode(b, true);
        if (args.adaptiveScan) {
            verticalRleNoModel = rleEncode(b, false);
        }
        if (args.model) {
            vector<vector<Pixel>> horizontalAfterModel = applyModel(b, true);
            if (args.adaptiveScan) {
                vector<vector<Pixel>> verticalAfterModel = applyModel(b, false);
                verticalRleWithModel = rleEncode(verticalAfterModel, false);
            }
            horizontalRleWithModel = rleEncode(horizontalAfterModel, true);
        }
        int compResult = compareCompressions(b, horizontalRleNoModel, horizontalRleWithModel, verticalRleNoModel, verticalRleWithModel);
        switch (compResult) {
            case 1:
                info.compressed = false;
                info.horizontal = true;
                info.model = false;
                break;
            case 2:
                info.compressed = true;
                info.horizontal = true;
                info.model = false;
                info.rleForBestOption = horizontalRleNoModel;
                break;
            case 3:
                info.compressed = true;
                info.horizontal = true;
                info.model = true;
                info.rleForBestOption = horizontalRleWithModel;
                break;
            case 4:
                info.compressed = true;
                info.horizontal = false;
                info.model = false;
                info.rleForBestOption = verticalRleNoModel;
                break;
            case 5:
                info.compressed = true;
                info.horizontal = false;
                info.model = true;
                info.rleForBestOption = verticalRleWithModel;
                break;
            default:
                break;
        }
        infos.push_back(info);
    }
    return infos;
}

vector<vector<vector<Pixel>>> splitIntoBlocks(vector<vector<Pixel>> &grid, CommandArgs &args) {
    vector<vector<vector<Pixel>>> blocks = {};
    int gridWidth = grid[0].size();
    int gridHeight = grid.size();
    int widthBlocks = gridWidth / BLOCK_SIZE;
    int heightBlocks = gridHeight / BLOCK_SIZE;
    if (args.adaptiveScan) {
        for (int h = 0; h < heightBlocks; h++) {
            for (int w = 0; w < widthBlocks; w++) {
                vector<vector<Pixel>> block = {};
                for (int i = 0; i < BLOCK_SIZE; i++) {
                    vector row(grid[h * BLOCK_SIZE + i].begin() + w * BLOCK_SIZE, grid[h * BLOCK_SIZE + i].begin() + w * BLOCK_SIZE + BLOCK_SIZE);
                    block.push_back(row);
                }
                blocks.push_back(block);
            }
        }
    }
    else {
        blocks.push_back(grid);
    }
    return blocks;
}

vector<vector<Pixel>> applyModel(vector<vector<Pixel>> &block, bool horizontal) {
    vector<vector<Pixel>> blockAfterModel = block;
    Pixel prev = block[0][0];
    if (horizontal) {
        for (size_t i = 0; i < block.size(); i++) {
            for (size_t j = 0; j < block[i].size(); j++) {
                if (i == 0 && j == 0) {
                    continue;
                }
                Pixel tempPrev = block[i][j];
                blockAfterModel[i][j] = block[i][j] - prev;
                prev = tempPrev;
            }
        }
    }
    else {
        for (size_t i = 0; i < block[0].size(); i++) {
            for (size_t j = 0; j < block.size(); j++) {
                if (i == 0 && j == 0) {
                    continue;
                }
                Pixel tempPrev = block[j][i];
                blockAfterModel[j][i] = block[j][i] - prev;
                prev = tempPrev;
            }
        }
    }
    return blockAfterModel;
}

vector<vector<Pixel>> getGrid(CommandArgs &args) {

    vector<vector<Pixel>> grid = {};
    vector<Pixel> row = {};
    FILE *fp = fopen(args.inputFile.c_str(), "rb");
    if (fp == nullptr) {
        cerr << "Unable to open file\n";
        exit(1);
    }
    int c;
    int widthCnt = 0;
    while ((c = fgetc(fp)) != EOF) {
        row.push_back(c);
        widthCnt++;
        if (widthCnt == args.width) {
            grid.push_back(row);
            row = {};
            widthCnt = 0;
        }
    }
    fclose(fp);
    return grid;
}

unordered_map<Pixel, int> getSymbolFrequencies(vector<BlockInfo> &info) {
    unordered_map<Pixel, int> symbolFrequencies;
    for (auto block : info) {
        if (!block.compressed) {
            for (auto blockRow : block.originalBlock) {
                for (auto blockCol: blockRow) {
                    symbolFrequencies[blockCol]++;
                }
            }
        }
        else {
            for (auto t : block.rleForBestOption) {
                symbolFrequencies[get<0>(t)]++;
                symbolFrequencies[get<1>(t)]++;
            }
        }
    }

    return symbolFrequencies;
}

vector<tuple<Pixel, int>> getCodeLengths(unordered_map<Pixel, int> &symbolFrequencies) {
    vector<int> frequencies = {};
    for (auto s : symbolFrequencies) {
        frequencies.push_back(s.second);
    }
    int numberOfFrequencies = frequencies.size();
    vector<int> hr(2 * numberOfFrequencies);
    vector<int> codeLengths(numberOfFrequencies);
    vector<tuple<int, int>> hufHeap = {};

    make_heap(hufHeap.begin(), hufHeap.end());
    for (int i = 0; i < numberOfFrequencies; i++) {
        tuple<int, int> t = make_pair(frequencies[i], numberOfFrequencies + i);
        hufHeap.push_back(t);
        push_heap(hufHeap.begin(), hufHeap.end(), greater<tuple<int, int>>());
    }

    // Hirschberg-Siemenski
    while (numberOfFrequencies > 1) {
        pop_heap(hufHeap.begin(), hufHeap.end(), greater<tuple<int, int>>());
        tuple<int, int> a = hufHeap.back();
        hufHeap.pop_back();

        pop_heap(hufHeap.begin(), hufHeap.end(), greater<tuple<int, int>>());
        tuple<int, int> b = hufHeap.back();
        hufHeap.pop_back();

        numberOfFrequencies -= 1;
        hr[get<1>(a)] = hr[get<1>(b)] = numberOfFrequencies;

        tuple<int, int> temp = make_pair(get<0>(a) + get<0>(b), numberOfFrequencies);
        hufHeap.push_back(temp);
        push_heap(hufHeap.begin(), hufHeap.end(), greater<tuple<int, int>>());
    }

    for (size_t i = 0; i < frequencies.size(); i++) {
        int j = hr[frequencies.size() + i];
        int l = 1;
        while (j > 1) {
            j = hr[j];
            l++;
        }
        codeLengths[i] = l;
    }

    vector<tuple<Pixel, int>> symbolsWithCodeLengths = {};
    int i = 0;
    for (auto s : symbolFrequencies) {
        tuple<Pixel, int> t = make_pair(s.first, codeLengths[i]);
        symbolsWithCodeLengths.push_back(t);
        i++;
    }

    // Sort the sequence in ascending order by the code length
    sort(symbolsWithCodeLengths.begin(), symbolsWithCodeLengths.end(), [](const tuple<Pixel, int> a, tuple<Pixel, int> b) {
        return get<1>(a) < get<1>(b);
    });

    return symbolsWithCodeLengths;
}

unordered_map<Pixel, string> getCodeFromLengths(vector<tuple<Pixel, int>> &symbolsWithCodeLengths) {
    unordered_map<Pixel, string> huffmanCode;
    vector<int> deltas(symbolsWithCodeLengths.size());

    deltas[0] = 0;
    for (size_t i = 1; i < deltas.size(); i++) {
        int x = get<1>(symbolsWithCodeLengths[i]);
        int y = get<1>(symbolsWithCodeLengths[i - 1]);
        deltas[i] = x - y;
    }
    int prev = 0;
    string bit = "";
    for (int i = 0; i < get<1>(symbolsWithCodeLengths[0]); i++) {
        bit += "0";
    }
    huffmanCode[get<0>(symbolsWithCodeLengths[0])] = bit;
    for (size_t i = 1; i < symbolsWithCodeLengths.size(); i++) {
        bit = "";
        int bitsAsNumber = (prev + 1) << (deltas[i]);
        prev = bitsAsNumber;
        bitset<32> codeInBits(bitsAsNumber);
        bit = codeInBits.to_string();

        // Remove unnecessary zeros
        while ((int) bit.size() != get<1>(symbolsWithCodeLengths[i])) {
            bit.erase(0, 1);
        }
        huffmanCode[get<0>(symbolsWithCodeLengths[i])] = bit;
    }
    return huffmanCode;
}

// Returns canonical Huffman code together with code lengths
tuple<unordered_map<Pixel, string>, vector<tuple<Pixel, int>>> getCanonicalHuffmanCode(vector<BlockInfo> &info) {
    unordered_map<Pixel, int> symbolFrequencies = getSymbolFrequencies(info);

    // Adding EOF symbol
    symbolFrequencies[256] = 1;
    vector<tuple<Pixel, int>> symbolsWithCodeLengths = getCodeLengths(symbolFrequencies);

    tuple<unordered_map<Pixel, string>, vector<tuple<Pixel, int>>> codesAndLengths =
        make_pair(getCodeFromLengths(symbolsWithCodeLengths), symbolsWithCodeLengths);

    return codesAndLengths;
}

void writeBlocksInfo(vector<BlockInfo> &info, FILE *fp) {
    int bitsUsed = 0;
    u_char blockTriple = 0;

    // For each block write if it is compressed and how it is traversed
    for (auto b : info) {
        if (bitsUsed == 8) {
            fputc(blockTriple, fp);
            bitsUsed = 0;
            blockTriple = 0;
        }
        blockTriple |= (b.compressed << (7 - bitsUsed));
        bitsUsed++;
        if (bitsUsed == 8) {
            fputc(blockTriple, fp);
            bitsUsed = 0;
            blockTriple = 0;
        }
        blockTriple |= (b.horizontal << (7 - bitsUsed));
        bitsUsed++;
        if (bitsUsed == 8) {
            fputc(blockTriple, fp);
            bitsUsed = 0;
            blockTriple = 0;
        }
        blockTriple |= (b.model << (7 - bitsUsed));
        bitsUsed++;
        if (bitsUsed == 8) {
            fputc(blockTriple, fp);
            bitsUsed = 0;
            blockTriple = 0;
        }
    }
    if (bitsUsed % 8 != 0) {
        fputc(blockTriple, fp);
    }
}

void writeStringToFile(string str, FILE *fp, int &bitsUsed, int &byteToWrite) {
    for (u_char c1: str) {
        int bit = c1 == '1' ? 1 : 0;
        byteToWrite |= (bit << (7 - bitsUsed));
        bitsUsed++;
        if (bitsUsed == 8) {
            fputc(byteToWrite, fp);
            bitsUsed = 0;
            byteToWrite = 0;
        }
    }
    if (bitsUsed == 8) {
        fputc(byteToWrite, fp);
        bitsUsed = 0;
        byteToWrite = 0;
    }
}

void writeData(vector<BlockInfo> &info, unordered_map<Pixel, string> &canonicalHuffman, FILE *fp) {
    int bitsUsed = 0;
    int byteToWrite = 0;
    for (auto b : info) {
        if (b.compressed) {
            for (auto x : b.rleForBestOption) {
                string x1 = canonicalHuffman[get<0>(x)];
                string x2 = canonicalHuffman[get<1>(x)];

                writeStringToFile(x1, fp, bitsUsed, byteToWrite);
                writeStringToFile(x2, fp, bitsUsed, byteToWrite);
                
            }
        }
        else {
            for (auto x : b.originalBlock) {
                for (auto y : x) {
                    string y1 = canonicalHuffman[y];
                    writeStringToFile(y1, fp, bitsUsed, byteToWrite);
                }
            }
        }
    }
    string end = canonicalHuffman[256];
    writeStringToFile(end, fp, bitsUsed, byteToWrite);
    if (bitsUsed % 8 != 0) {
        fputc(byteToWrite, fp);
    }
}

void writeHuffmanCode(vector<tuple<Pixel, int>> &canonicalHuffman, FILE *fp) {
    for (auto entry : canonicalHuffman) {
        u_char symbolBytes[2] = {};
        u_char codeBytes[2] = {};
        Pixel symb = get<0>(entry);
        Pixel codeLen = get<1>(entry);

        for (int i = 0; i < 2; i++) {
            symbolBytes[i] = (symb >> ((1 - i) * 8)) & 0xFF;
            codeBytes[i] = (codeLen >> ((1 - i) * 8)) & 0xFF;
        }
        fwrite(symbolBytes, 1, 2, fp);
        fwrite(codeBytes, 1, 2, fp);
    }
}

void writeCompressedToFile(CommandArgs &args, vector<BlockInfo> &info, tuple<unordered_map<Pixel, string>, vector<tuple<Pixel, int>>> &canonicalHuffman) {
    FILE *fp = fopen(args.outputFile.c_str(), "wb");
    if (fp == nullptr) {
        cerr << "Unable to open file\n";
        exit(1);
    }
    u_char bytesWidth[4] = {};
    u_char numOfBlocks[4] = {};
    u_char differentSymbols[4] = {};
    int32_t infoSize = (int32_t) info.size();
    int32_t numOfDifferentSymbols = (int32_t) get<0>(canonicalHuffman).size();
    for (int i = 0; i < 4; i++) {
        bytesWidth[i] = (args.width >> ((3 - i) * 8)) & 0xFF;
        numOfBlocks[i] = (infoSize >> ((3 - i) * 8)) & 0xFF;
        differentSymbols[i] = (numOfDifferentSymbols >> ((3 - i) * 8)) & 0xFF; 
    }
    // Write image width
    fwrite(bytesWidth, 1, 4, fp);
    // Write total number of different symbols
    fwrite(differentSymbols, 1, 4, fp);
    // Write total number of blocks
    fwrite(numOfBlocks, 1, 4, fp);

    writeHuffmanCode(get<1>(canonicalHuffman), fp);
    writeBlocksInfo(info, fp);
    writeData(info, get<0>(canonicalHuffman), fp);

    fclose(fp);
}

void compressFile(CommandArgs &args) {
    vector<vector<Pixel>> grid = getGrid(args);
    vector<vector<vector<Pixel>>> blocks = splitIntoBlocks(grid, args);

    vector<BlockInfo> info = findBestCompression(blocks, args);

    tuple<unordered_map<Pixel, string>, vector<tuple<Pixel, int>>> canonicalHuffman = getCanonicalHuffmanCode(info);

    writeCompressedToFile(args, info, canonicalHuffman);
}