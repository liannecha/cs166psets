#include "NaiveHyperLogLog.h"
using namespace std;

NaiveHyperLogLog::NaiveHyperLogLog(HashFunction fn, uint8_t precision) {
    /* TODO: Delete this comment and the next two lines, then implement
     * this function.
     */
    (void) fn;
    (void) precision;
}

void NaiveHyperLogLog::see(uint64_t value) {
    /* TODO: Delete this comment and the next line, then implement
     * this function.
     */
    (void) value;
}

uint64_t NaiveHyperLogLog::estimate() const {
    /* TODO: Delete this comment and the next line, then implement
     * this function.
     */
    return -1;
}

/* * * * * Test Cases Below This Point * * * * */

#include "SimpleTest/SimpleTest.h"

PROVIDED_TEST("NaiveHyperLogLog: Errors on invalid inputs.") {
    EXPECT_ERROR(NaiveHyperLogLog(fnv1a(), 0), std::invalid_argument);
    EXPECT_ERROR(NaiveHyperLogLog(fnv1a(), 1), std::invalid_argument);
    EXPECT_ERROR(NaiveHyperLogLog(fnv1a(), 2), std::invalid_argument);
    EXPECT_ERROR(NaiveHyperLogLog(fnv1a(), 13), std::invalid_argument);
    EXPECT_ERROR(NaiveHyperLogLog(fnv1a(), 17), std::invalid_argument);
}

PROVIDED_TEST("NaiveHyperLogLog: Output correct when no items seen.") {
    /* Range over all possible choices of precision. For each one, with all
     * counters set to zero, the harmonic mean will be
     *
     * m / (2^0 + 2^0 + ... + 2^0) [m times] = 1
     *
     * Therefore, the estimate will be m * 1 = 1.
     */
    for (uint64_t i = 3; i <= 12; i++) {
        NaiveHyperLogLog nhll(fnv1a(), i); // Choice of hash doesn't matter
        EXPECT_EQUAL(nhll.estimate(), 1ULL << i);
    }
}

PROVIDED_TEST("NaiveHyperLogLog: Output correct on simple nonempty set.") {
    /* We use a precision of 8 so that there are 256 counters. This makes the
     * first byte of each hash used as the counter index. To control what
     * lands where, we use the identity hash function.
     */
    NaiveHyperLogLog nhll(identity(), 8);
    EXPECT_EQUAL(nhll.estimate(), 256);   // 2^8 * harmonic mean of 2^8 copies of 1
    
    /* Range across all counters, setting each to 1. */
    for (uint64_t byte = 0; byte < 256; byte++) {
        /* This number has the given byte as its first eight bits and a 1 LSB.
         * This should update each counter to be 1.
         */
        nhll.see(1ULL + (byte << 56UL));
    }
    
    EXPECT_EQUAL(nhll.estimate(), 512); // 2^8 * harmonic mean of 2^8 copies of 2.
    
    /* Should have no effect; we've already seen all of these. */
    for (uint64_t byte = 0; byte < 256; byte++) {
        /* This number has the given byte as its first eight bits and a 1 LSB.
         * This should update each counter to be 1.
         */
        nhll.see(1ULL + (byte << 56UL));
    }
    
    EXPECT_EQUAL(nhll.estimate(), 512);
}

PROVIDED_TEST("NaiveHyperLogLog: Updates counters appropriately.") {
    /* We use a precision of 8 so that there are 256 counters. This makes the
     * first byte of each hash used as the counter index. To control what
     * lands where, we use the identity hash function.
     */
    NaiveHyperLogLog nhll(identity(), 8);
    EXPECT_EQUAL(nhll.estimate(), 256);   // 2^8 * harmonic mean of 2^8 copies of 1
    
    /* Range across all counters, setting each to 1. */
    for (uint64_t byte = 0; byte < 256; byte++) {
        /* This number has the given byte as its first eight bits and a 1 LSB.
         * This should update each counter to be 1.
         */
        nhll.see(1ULL + (byte << 56UL));
    }
    
    EXPECT_EQUAL(nhll.estimate(), 512); // 2^8 * harmonic mean of 2^8 copies of 2.
    
    /* Now set each counter to 2. */
    for (uint64_t byte = 0; byte < 256; byte++) {
        /* Least-set 1 bit is now at position 1, zero-indexed. */
        nhll.see(2ULL + (byte << 56UL));
    }
    
    EXPECT_EQUAL(nhll.estimate(), 1024); // 2^8 * harmonic mean of 2^8 copies of 4
    
    /* Should have no effect; counters already have larger values. */
    for (uint64_t byte = 0; byte < 256; byte++) {
        nhll.see(1ULL + (byte << 56UL));
    }
    
    EXPECT_EQUAL(nhll.estimate(), 1024); // 2^8 * harmonic mean of 2^8 copies of 4
}

PROVIDED_TEST("NaiveHyperLogLog: Handles mixed counter values.") {
    /* We use a precision of 8 so that there are 256 counters. This makes the
     * first byte of each hash used as the counter index. To control what
     * lands where, we use the identity hash function.
     */
    NaiveHyperLogLog nhll(identity(), 8);
    EXPECT_EQUAL(nhll.estimate(), 256);   // 2^8 * harmonic mean of 2^8 copies of 1
    
    /* Set the first half of the counters equal to 1 and the second half to 2. */
    for (uint64_t byte = 0; byte < 256; byte++) {
        uint64_t offset = (byte >= 128? 1 : 2);
        nhll.see(offset + (byte << 56UL));
    }
    
    /* Harmonic mean of 128 copies of 2 and 128 copies of 4 is
     * 256 / (128 / 2 + 128 / 4) = 8/3
     *
     * Estimate is then 256 * 8 / 3 = 682.66, rounding down to 682.
     */
    EXPECT_EQUAL(nhll.estimate(), 682);
}

PROVIDED_TEST("NaiveHyperLogLog: Handles variable precision values.") {
    /* Top 3 bits of each hash identify counter. */
    NaiveHyperLogLog nhll(identity(), 3);
    EXPECT_EQUAL(nhll.estimate(), 8); // 2^3 * harmonic mean of 2^3 copies of 1
    
    /* Make the counters equal to 1, 2, 4, 8, ..., 256. */
    for (uint64_t bit = 0; bit < 8; bit++) {
        /* First three bits are the bit pattern itself; last eight bits are 1s at
         * the (bit)th position.
         */
        nhll.see((bit << 61ULL) + (1ULL << bit));
    }
    
    /* Harmonic mean is
     * 8 / (1/2 + 1/4 + 1/8 + ... + 1/256) = 2048 / 255
     *
     * Estimate is then 8 * 2048 / 255 = 16384 / 255 ~= 64.25098, rounding down to 64.
     */
    EXPECT_EQUAL(nhll.estimate(), 64);
}

#include <map>

PROVIDED_TEST("NaiveHyperLogLog: Correct with FNV-1A hash on various set sizes.") {
    const uint64_t kStep       = 100000;
    
    /* Expected outcomes computed using a reference solution. */
    const map<uint64_t, uint64_t> kExpected = {
        {    100000,     88228 },
        {    200000,    119439 },
        {    300000,    129165 },
        {    400000,    140482 },
        {    500000,    153698 },
        {    600000,    169592 },
        {    700000,    187341 },
        {    800000,    206206 },
        {    900000,    215474 },
        {   1000000,    223579 },
    };
    
    NaiveHyperLogLog nhll(fnv1a(), 12);
    
    /* See the values 0, 1, 2, ..., etc., periodically pausing to
     * check that the values match the reference.
     */
    uint64_t actualN = 0;
    for (uint64_t i = 0; i < 10; i ++) {
        for (uint64_t j = 0; j < kStep; j++) {
            nhll.see(i * kStep + j);
            actualN++;
        }
        EXPECT_EQUAL(nhll.estimate(), kExpected.at(actualN));
    }
}
