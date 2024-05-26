#include "common.hpp"

CommandArgs parseArguments(int argc, char *argv[]) {
    CommandArgs args;
    int opt;
    char *eptr;
    while ((opt = getopt(argc, argv, "cdmai:o:w:h")) != -1) {
        switch (opt) {
            case 'c':
                args.mode = COMPRESS;
                break;
            
            case 'd':
                args.mode = DECOMPRESS;
                break;
            
            case 'm':
                args.model = true;
                break;

            case 'a':
                args.adaptiveScan = true;
                break;

            case 'i':
                args.inputFile = optarg;
                break;

            case 'o':
                args.outputFile = optarg;
                break;

            case 'w':
                args.width = (int) strtol(optarg, &eptr, 10);
                break;

            case 'h':
                args.help = true;
                break;
        }
    }
    return args;
}

void printHelp() {
    printf("Program options:\n");
    printf("\t -c compress\n");
    printf("\t -d decompress\n");
    printf("\t -i <file> input file\n");
    printf("\t -o <file> output file\n");
    printf("\t -m applies pixel difference model to input file\n");
    printf("\t -a applies adaptive scan\n");
    printf("\t -w <width> width of the image (only when compressing)\n");
    printf("\t -h show help\n");
}