#include "common.hpp"
#include "compress.hpp"
#include "decompress.hpp"

int main(int argc, char *argv[]) {

    CommandArgs args = parseArguments(argc, argv);
    if (args.help) {
        printHelp();
    }
    else if (args.mode == COMPRESS) {
        compressFile(args);
    }
    // Decompress
    else {
        decompressFile(args);
    }
    return 0;
}