/******************************************************************************
 * File: SuccinctRank.h
 *
 * An implementation of Jacobson's succinct rank data structure.
 */

#pragma once

#include "IntArray.h"

class SuccinctRank {
public:
    /* Constructs a succinct rank structure over an array of bits. The array
     * begins at the byte pointed at by bits and extends numBits bits
     * forward.
     *
     * You are guaranteed that numBits will never be 0, but it may be 1.
     *
     * You are guaranteed that bits is not a null pointer.
     *
     * You are guaranteed that the array pointed at by bits is allocated
     * in a way that's aligned to a multiple of 64 bits, so the
     * bitAt, integerAt, and writeIntegerAt functions will work on the
     * bit array.
     */
    SuccinctRank(const void* bits, std::uint64_t numBits);
    
    /* Computes a rank query at the given index. Specifically, computes the
     * sum of all bits up to but not including the given index, or,
     * equivalently, the number of 1 bits before the given index. So, for
     * example, rank(0) is always 0 (there are no bits before position 0)
     * and rank(length of the array) is the number of 1 bits in the array.
     *
     * You do not need to handle the case where bitIndex is out of range
     * for the bit array. Note, though, that the length of the bit
     * array is a valid index.
     */
    std::uint64_t rank(std::uint64_t bitIndex) const;
    
    
private:
    /* TODO: Implement this type however you see fit. We recommend making
     * strategic use of the IntArray type. Once you've implemented this,
     * remove this TODO comment.
     */
    const void* bits;
    uint64_t minBits;   // cutoff for, if numbits is below this, we just do a O(n) scan to answer rank
    uint64_t numBits;
    uint64_t blockSize;
    uint64_t miniBlockSize;
    uint64_t numBlocks;
    uint64_t numMiniInBlock;

    IntArray blockPrefixSums;
    IntArray miniBlockPrefixSums;
};
