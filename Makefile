CC=g++
CPPFLAGS=-std=c++20 -Wall

all: huff_codec.cpp compress.o decompress.o common.o
	$(CC) $(CPPFLAGS) huff_codec.cpp compress.o common.o decompress.o -o huff_codec

compress.o: compress.cpp
	$(CC) $(CPPFLAGS) compress.cpp -c -o compress.o

decompress.o: decompress.cpp
	$(CC) $(CPPFLAGS) decompress.cpp -c -o decompress.o

common.o: common.cpp
	$(CC) $(CPPFLAGS) common.cpp -c -o common.o

clean:
	rm *.o huff_codec