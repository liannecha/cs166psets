/**
 * File: Hashes.cpp
 * Author: Keith Schwarz (htiek@cs.stanford.edu),
 *         Kevin Gibbons
 *
 * Implementation of several different hash functions. The implementation of
 * tabulation hashing provided here was originally written by Kevin Gibbons
 * when he was a CS166 TA.
 */
#include "Hashes.h"
#include <cstdlib>
#include <cstdint>
#include <array>
#include <random>
#include <climits>

/* FNV-1A Hashing. Check Wikipedia for details. */
namespace {
    const std::uint64_t kFNVPrime = 1099511628211ULL;
    const std::uint64_t kFNVBasis = 14695981039346656037ULL;
    
    std::uint64_t fnv1aHashBytes(const std::uint8_t* bytes, std::uint64_t numBytes) {
        std::uint64_t result = kFNVBasis;
        
        for (std::uint64_t i = 0; i < numBytes; i++) {
            result *= kFNVPrime;
            result ^= bytes[i];
        }
        
        return result;
    }
}

HashFunction fnv1a() {
    return [] (std::uint64_t value) {
        return fnv1aHashBytes(reinterpret_cast<const std::uint8_t*>(&value), sizeof value);
    };
}

/* Tabulation hashing. */
namespace {
    using TabulationTable = std::array<std::array<std::uint64_t, 256>, 8>;

    TabulationTable tabulationTable(std::uint64_t seed) {
        std::mt19937 engine(seed);
        std::uniform_int_distribution<std::uint64_t> dist;

        TabulationTable table;
        for (std::size_t i = 0; i < 8; i++) {
            for (std::size_t byte = 0; byte < 256; byte++) {
                table[i][byte] = dist(engine);
            }
        }
        return table;
    }
}

HashFunction tabulation(std::uint64_t seed) {
    auto table = tabulationTable(seed);
    return [=] (std::uint64_t key) {
        std::uint64_t result = 0;
        for (std::uint64_t i = 0; i < 8; i++) {
            result ^= table[i][(key >> (i * 8)) & 0xFF];
        }
        return result;
    };
}

/* Identity function; terrible hash function to use in practice but great for testing. */
HashFunction identity() {
    return [=] (std::uint64_t value) {
        return value;
    };
}
