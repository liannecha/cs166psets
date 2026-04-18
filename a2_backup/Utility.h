/******************************************************************************
 * File: Utility.h
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * Collection of useful helper functions for implementing a succinct rank
 * structure. These are provided to you. You are not likely to use all of
 * these.
 */

#pragma once

#include <cstddef>
#include <cstdint>

/* Computes ceiling(numerator / denominator). */
std::uint64_t divideAndRoundUp(std::uint64_t numerator, std::uint64_t denominator);

/* From A0: Given a bit sequence and an index, returns the bit at that index. */
int bitAt(const void* bitSequence, std::uint64_t bitIndex);

/* From A0: Given a bit sequence, a bit start index, and a number of bits to read, reads
 * an integer of that many bits from that position. Throws a std::invalid_argument
 * exception if the number of bits is bigger than 64.
 */
std::uint64_t integerAt(const void* bitSequence, std::uint64_t startIndex, std::uint8_t numBits);

/* From A0: Given a bit sequence, a bit start index, and a number of bits to write, writes
 * an integer of that many bits from that position. Throws a std::invalid_argument
 * exception if the number of bits is bigger than 64.
 */
void writeIntegerAt(void* bitSequence, std::uint64_t startIndex, std::uint8_t numBits, std::uint64_t bitsToWrite);

/* Nicely-behaved allocators that always round up to a multiple of 64 bits. This is useful
 * in conjunction with integerAt and writeIntegerAt, since those functions assume everything
 * is allocated in nice 64-bit chunks.
 */
void* allocate(std::uint64_t numBytes);
void  deallocate(void* memory);
