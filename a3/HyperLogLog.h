/******************************************************************************
 * File: HyperLogLog.h
 *
 * An implementation of a traditional HyperLogLog estimator with a correction
 * for alpha_m.
 */

#pragma once

#include "Utility/CardinalityEstimator.h"
#include "Hashes.h"

/* Base type is here so that we can look up the type by name at runtime. You
 * won't need to look at the base class.
 */
class HyperLogLog: public CardinalityEstimator<HyperLogLog> {
public:
    /* Constructs an estimator with the given value of p that uses
     * the specified hash function. Refer to Hashes.h for details
     * about the HashFunction type.
     *
     * Recall that m is chosen to be 2^p. p must be between 3 and
     * 12, inclusive, and you should throw a std::invalid_argument
     * exception if this is not the case.
     */
    explicit HyperLogLog(HashFunction fn, std::uint8_t precision);
    
    /* Records that we've seen the specified value. */
    void see(std::uint64_t value);
    
    /* Estimates the cardinality of the set seen thus far. */
    std::uint64_t estimate() const;
    
private:
    /* TODO: Delete this comment and implement this type. */
};
