/**
 * File: Hashes.h
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * Header file exporting a variety of hash functions.
 */
#pragma once

#include <functional>
#include <cstdint>

/* Type representing a hash function. A hash function is a function that
 * can be called on a 64-bit integer input and that produces a 64-bit
 * integer as output.
 *
 * You can use this type as follows:
 *
 *    HashFunction fn = // ... some initialization
 *    uint64_t result = fn(137);
 */
using HashFunction = std::function<uint64_t (std::uint64_t)>;

/* Returns the identity function. This is a _terrible_ hash function
 * that no one would actually use in practice, but it's great for
 * testing purposes because we can control what lands here.
 */
HashFunction identity();

/* Returns a tabulation hash function given the specified random
 * seed. A tabulation hash function works by breaking the input
 * apart into 8-bit blocks. Each block is associated with a table
 * of 256 64-bit random values. The entries for each byte are then
 * XORed together. This function has excellent theoretical properties;
 * see "The Power of Simple Tabulation Hashing" by Pǎtraşcu and Thorup
 * for details.
 */
HashFunction tabulation(std::uint64_t seed = 137);

/* Returns the 64-bit FNV-1A hash function. This function has good
 * dispersion of random bits over a wide range and is fast to compute.
 */
HashFunction fnv1a();
