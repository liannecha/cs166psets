#include "SuccinctRank.h"
#include "Utility.h"
#include "SimpleTest/SimpleTest.h"
#include "SimpleTest/MemoryDiagnostics.h"
using namespace std;

PROVIDED_TEST("SuccinctRank: Rank within a single 64-bit block.") {
    const uint64_t block = 0xAAAAAAAAAAAAAAAA;
    
    SuccinctRank ranker(&block, 64);
    
    int trueCount = 0;
    for (int i = 0; i < 64; i++) {
        EXPECT_EQUAL(ranker.rank(i), trueCount);
        
        /* Every other bit is a 1. */
        if (i % 2 == 1) {
            trueCount++;
        }
    }
 
    EXPECT_EQUAL(ranker.rank(64), trueCount);
}

PROVIDED_TEST("SuccinctRank: Rank across multiple a single 64-bit blocks.") {
    const uint64_t blocks[] = {
        0xAAAAAAAAAAAAAAAA,
        0xAAAAAAAAAAAAAAAA
    };
    
    SuccinctRank ranker(blocks, 128);
    
    int trueCount = 0;
    for (int i = 0; i < 128; i++) {
        EXPECT_EQUAL(ranker.rank(i), trueCount);
        
        /* Every other bit is a 1. */
        if (i % 2 == 1) {
            trueCount++;
        }
    }
 
    EXPECT_EQUAL(ranker.rank(128), trueCount);
}

PROVIDED_TEST("SuccinctRank: Works on tiny bit arrays.") {
    const uint64_t block = 0xAAAAAAAAAAAAAAAA;
    
    for (int bits = 1; bits <= 64; bits++) {
        SuccinctRank ranker(&block, bits);
        
        int trueCount = 0;
        for (int i = 0; i < bits; i++) {
            EXPECT_EQUAL(ranker.rank(i), trueCount);
            
            /* Every other bit is a 1. */
            if (i % 2 == 1) {
                trueCount++;
            }
        }
     
        EXPECT_EQUAL(ranker.rank(bits), trueCount);
    }
}

PROVIDED_TEST("SuccinctRank: Works on large, varied bit arrays.") {
    const uint64_t kNumBytes = 100000;
    static_assert(kNumBytes % 8 == 0, "Bytes must be a mutiple of 8.");
    
    uint8_t* bytes = static_cast<uint8_t*>(allocate(kNumBytes));
    
    /* Ensure all memory is cleaned up even if the test fails. */
    try {
        /* Fill the bytes with a nice pattern. */
        for (uint64_t i = 0; i < kNumBytes; i++) {
            bytes[i] = i & 0xFF;
        }
        
        /* Confirm the answers are correct. */
        SuccinctRank ranker(bytes, kNumBytes * 8);
        
        uint64_t trueCount = 0;
        for (uint64_t bit = 0; bit < kNumBytes * 8; bit++) {
            EXPECT_EQUAL(ranker.rank(bit), trueCount);
            trueCount += bitAt(bytes, bit);
        }
        
        EXPECT_EQUAL(ranker.rank(kNumBytes * 8), trueCount);
    } catch (...) {
        deallocate(bytes);
        throw;
    }
    
    deallocate(bytes);
}
