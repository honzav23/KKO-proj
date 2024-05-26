import os
def test():
    dir = "./data/data"
    files = os.listdir(dir)
    
    for f in files:
        if f == "info.txt":
            continue
        os.system(f"./huff_codec -c -i {dir}/{f} -w 512 -o ./test.out")
        os.system(f"./huff_codec -d -i ./test.out -o ./test.output")
        os.system(f"diff ./test.output {dir}/{f}")

        # sizeInBits = os.path.getsize("./test.out") * 8
        # print(f"{f}: mode: none, bpc: {sizeInBits / 262144}")

        os.system(f"./huff_codec -c -m -i {dir}/{f} -w 512 -o ./test.out")
        os.system(f"./huff_codec -d -m -i ./test.out -o ./test.output")
        os.system(f"diff ./test.output {dir}/{f}")

        # sizeInBits = os.path.getsize("./test.out") * 8
        # print(f"{f}: mode: -m, bpc: {sizeInBits / 262144}")

        os.system(f"./huff_codec -c -a -i {dir}/{f} -w 512 -o ./test.out")
        os.system(f"./huff_codec -d -a -i ./test.out -o ./test.output")
        os.system(f"diff ./test.output {dir}/{f}")

        # sizeInBits = os.path.getsize("./test.out") * 8
        # print(f"{f}: mode: -a, bpc: {sizeInBits / 262144}")

        os.system(f"./huff_codec -c -m -a -i {dir}/{f} -w 512 -o ./test.out")
        os.system(f"./huff_codec -d -m -a -i ./test.out -o ./test.output")
        os.system(f"diff ./test.output {dir}/{f}")

        # sizeInBits = os.path.getsize("./test.out") * 8
        # print(f"{f}: mode: -m -a, bpc: {sizeInBits / 262144}")


if __name__ == "__main__":
    test()