#include "Utility.h"
#include <stdexcept>
#include <algorithm>
#include <cstdlib>
#include <unordered_map>
#include <string>
#include <cstring>
#include "SimpleTest/MemoryDiagnostics.h"
using namespace std;

uint64_t divideAndRoundUp(uint64_t numerator, uint64_t denominator) {
    if (denominator == 0) {
        throw invalid_argument("Can't divide by zero.");
    }
    
    auto result = numerator / denominator;
    if (numerator % denominator != 0) result++;
    
    return result;
}

int bitAt(const void* bitSequence, std::uint64_t bitIndex) {
    const uint8_t* blocks = static_cast<const uint8_t*>(bitSequence);
    
    auto byteIndex  = bitIndex / 8;
    auto byteOffset = bitIndex % 8;
    return (blocks[byteIndex] & (1 << byteOffset)) >> byteOffset; 
}

uint64_t integerAt(const void* bitSequence, std::uint64_t startIndex, std::uint8_t numBits) {
    if (numBits > 64) {
        throw runtime_error("Invalid argument.");
    }
    if (bitSequence == nullptr) {
        throw runtime_error("Null pointer.");
    }
    
    /* Break apart into 64-bit blocks. */
    auto i64s = static_cast<const uint64_t *>(bitSequence);
    
    /* Index and offset. */
    auto blockIndex  = startIndex / 64;
    auto blockOffset = startIndex % 64;
    
    /* See how many bits we're getting from the first number. That's the min of
     * "all the remaining ones" and "what we need."
     */
    auto fromFirst = min(64 - blockOffset, uint64_t(numBits));
    
    /* Read that many bits. To do so, mask off everything above us (if there
     * is anything) and then shift us down by the relative offset.
     */
    uint64_t lowBits = i64s[blockIndex];
    
    /* If there is anything after these bits, then we need to mask all of that
     * off. The easiest way to do this is to shift upward to drop off everything
     * except the bits we want, then shift back down to capture the bits we want.
     *
     * The index of the bit after us is blockOffset + fromFirst. We want to drop
     * everything after that, so we shift by 64 - blockOffset - fromFirst.
     */
    lowBits <<= (64 - blockOffset - fromFirst);
    
    /* We now occupy the fromFirst highest bits of the number, so shift us back
     * down.
     */
    lowBits >>= (64 - fromFirst);
    
    /* We may need more bits, or maybe we don't. If we don't, great! We're done. */
    if (fromFirst == numBits) {
        return lowBits;
    }
    
    /* We need more bits. Extract the remaining from the next number using the same
     * technique as above. The number of remaining bits is given by numBits - fromFirst,
     * but since we begin at the start index of the block, we don't need to shift down.
     * We just shift up and back.
     */
    uint64_t highBits = i64s[blockIndex + 1];
    highBits <<= (64 - numBits + fromFirst);
    highBits >>= (64 - numBits + fromFirst);
    
    /* Shift them together. */
    return (highBits << fromFirst) + lowBits;
}


void writeIntegerAt(void* bitSequence, uint64_t startIndex, uint8_t numBits, uint64_t bitsToWrite) {
    if (numBits > 64) {
        throw runtime_error("Invalid argument.");
    }
    if (bitSequence == nullptr) {
        throw runtime_error("Null pointer.");
    }
    
    /* Break apart into 64-bit blocks. */
    auto i64s = static_cast<uint64_t *>(bitSequence);
    
    /* Index and offset. */
    auto blockIndex  = startIndex / 64;
    auto blockOffset = startIndex % 64;
    
    /* See how many bits we're writing into the first number. That's the min of
     * "all the remaining ones" and "what we need."
     */
    auto fromFirst = min(64 - blockOffset, uint64_t(numBits));
    
    /* Form a bitmask that preserves all bits outside what we're writing and
     * which excludes all bits that we are writing. This means that we need
     * 1s at all positions that we're writing to.
     *
     * To do this, start by making an array of fromFirst 1 bits. We do this
     * by making a number that's all 1s, then shifting it up so that we drop
     * of all but fromFirst 1s. Then shift it down so that we overlap the
     * region to write. That means that the lowest 1 needs to be right on
     * top of the block offset. The lowest 1 is currently at position
     * 64 - fromFirst, so it needs to be shifted back 64 - fromFirst - blockOffset
     * bits down.
     */
    uint64_t lowMask = (uint64_t(-1) << (64 - fromFirst)) >> (64 - fromFirst - blockOffset);
    i64s[blockIndex] &= ~lowMask;
    
    /* Overwrite those bits by ORing in the bits we need to write. It might
     * appear that we should mask out some number of bits from bitsToWrite,
     * but we actually do NOT need to do that. Here's why. If we are writing
     * all the bits in, great! No masking is needed. If some of those bits
     * go in the next number, that's okay too - they get shifted off as we
     * reposition the bits.
     */
    
    /* Overwrite those bits by ORing in the bits we need to write. */
    i64s[blockIndex] |= (bitsToWrite << blockOffset);
    
    /* We may need more bits, or maybe we don't. If we don't, great! We're done. */
    if (fromFirst == numBits) {
        return;
    }
    
    /* We need more bits. We need to clear the low bits of the next number to
     * make room for our bits. Specifically, we need to write numBits - fromFirst
     * bits, so we'll form a mask in the same way that we did before.
     */
    uint64_t highMask = (uint64_t(-1) << (64 - numBits + fromFirst)) >> (64 - numBits + fromFirst);
    i64s[blockIndex + 1] &= ~highMask;
    
    /* Now, OR in the bits we didn't use thus far. */
    i64s[blockIndex + 1] |= bitsToWrite >> fromFirst;
}

namespace {
    auto& allocationTable() {
        static unordered_map<void*, uint64_t> theTable;
        return theTable;
    }
    
    struct Initializer {
        Initializer() {
            MemoryDiagnostics::registerSentinel(typeid(void*));
        }
    } init;
}

void* allocate(uint64_t numBytes) {
    if (numBytes == 0) {
        throw runtime_error("allocate(): Cannot allocate space for 0 bytes.");
    }
    
    /* Round up to the nearest multiple of eight bytes. */
    if (numBytes % 8 != 0) {
        numBytes += 8 - (numBytes % 8);
    }

    auto* result = malloc(numBytes);
    if (result == nullptr) {
        throw runtime_error("allocate(): Cannot allocate space for " + to_string(numBytes) + " bytes.");
    }
    memset(result, 0, numBytes);
    
    allocationTable()[result] = numBytes;
    MemoryDiagnostics::recordNew(typeid(void*));
    return result;
}

void deallocate(void* memory) {
    auto itr = allocationTable().find(memory);
    if (itr == allocationTable().end()) {
        throw runtime_error("deallocate() called on pointer that wasn't allocated.");
    }
    
    allocationTable().erase(itr);
    MemoryDiagnostics::recordDelete(typeid(void*));
    free(memory);
}
