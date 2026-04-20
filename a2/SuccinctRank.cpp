#include "SuccinctRank.h"
#include "Utility.h"       // You'll likely want what's in here.
#include <cmath>           // For log2, which we assume you'll need.
#include <iostream>
using namespace std;

SuccinctRank::SuccinctRank(const void* bits, uint64_t numBits) : bits(bits), numBits(numBits) {
    minBits = 4;

    // if numbits is small, just do O(n)=O(1) scan
    if (numBits <= minBits) {
        return;
    }

    // compute blocksizes
    blockSize = log2(numBits) * log2(numBits);
    numBlocks = divideAndRoundUp(numBits, blockSize);
    miniBlockSize = log2(numBits) / 2;
    numMiniInBlock = divideAndRoundUp(blockSize, miniBlockSize);

    // compute prefix sums for top block
    // there can be at most numBits bits, so we need ceil(log2(numBits)) to properly store the prefix sums
    blockPrefixSums = IntArray(BitCount(ceil(log2(numBits + 1))), numBlocks + 1);
    uint64_t bitCount = 0;
    for (uint64_t i = 0; i < numBits; i++) {
        if (i % blockSize == 0) {
            blockPrefixSums[i / blockSize] = bitCount;
        }

        bitCount += bitAt(bits, i);
    }
    // store the total number of bits at the very end
    blockPrefixSums[numBlocks] = bitCount;

    // compute prefix sums for miniblocks
    // each block holds at most blockSize bits. There are numBlocks * (number of miniblocks in a block) miniblocks.
    miniBlockPrefixSums = IntArray(BitCount(ceil(log2(blockSize + 1))), numBlocks * numMiniInBlock);
    uint64_t miniBlock = 0;
    for (uint64_t block = 0; block < numBlocks; block++) {
        bitCount = 0;
        for (uint64_t i = block * blockSize; i < min(block * blockSize + blockSize, numBits); i++) {
            if ((i - block * blockSize) % miniBlockSize == 0) {
                miniBlockPrefixSums[miniBlock] = bitCount;
                miniBlock++;
            }

            bitCount += bitAt(bits, i);
        }
    }

    // init the four russians table. the table is 2^b by b, with each entry needing to store integers up to log(miniBlockSize)
    fourRussians = vector<IntArray>(1 << miniBlockSize, IntArray(BitCount(ceil(log2(miniBlockSize + 1))), miniBlockSize));
    // fill the four russians table using dp
    uint64_t topNumber = (1 << miniBlockSize) - 1;
    for (uint64_t number = 0; number <= topNumber; number++) {
        fourRussians[number][0] = 0;
    }

    for (uint64_t index = 1; index < miniBlockSize; index++) {
        for (uint64_t number = 0; number <= topNumber; number++) {
            fourRussians[number][index] = fourRussians[number][index - 1] + ((number >> (index - 1)) & 1);
        }
    }
}

uint64_t SuccinctRank::rank(std::uint64_t bitIndex) const {
    uint64_t bitCount = 0;
    if (numBits <= minBits) {
        for (uint64_t i = 0; i < bitIndex; i++) {
            bitCount += bitAt(bits, i);
        }

        return bitCount;
    }

    // if the user is querying the end, just return the total number of bits (stored at the end of the prefix sums array)
    if (bitIndex == numBits) {
        return blockPrefixSums[numBlocks];
    }

    uint64_t block = bitIndex / blockSize;
    bitCount += blockPrefixSums[block];
    uint64_t localMiniBlock = (bitIndex % blockSize) / miniBlockSize;
    uint64_t miniBlock =  localMiniBlock + block * numMiniInBlock;
    bitCount += miniBlockPrefixSums[miniBlock];

    uint64_t num = integerAt(bits, block * blockSize + localMiniBlock * miniBlockSize, miniBlockSize);
    uint64_t k = (bitIndex % blockSize) % miniBlockSize;
    bitCount += fourRussians[num][k];
    return bitCount;
}
