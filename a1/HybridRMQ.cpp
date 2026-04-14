#include "HybridRMQ.h"
#include "SimpleTest/SimpleTest.h"
using namespace std;

HybridRMQ::HybridRMQ(const RMQEntry* elems, size_t numElems) : elems(elems), numElems(numElems) {
    summaryRMQ = NULL;
    summary = NULL;

    // If the input array is empty, return.
    if (numElems == 1) {
        return;
    }

    // Compute block size.
    blockSize = log2(numElems);
    summary = new RMQEntry[(numElems-1)/blockSize + 1];
  
    // Compute the minimum for each block.
    for (size_t b = 0; b < (numElems - 1) / blockSize + 1; b++) {
        RMQEntry minElem = elems[b * blockSize];

        for (size_t i = b * blockSize; i < min(b * blockSize + blockSize, numElems); i++) {
            if (elems[i] < minElem) {
                minElem = elems[i];
            }
        }
        summary[b] = minElem;
    }
    summaryRMQ = new SparseTableRMQ(summary, (numElems - 1) / blockSize + 1);
}

HybridRMQ::~HybridRMQ() {
    delete summaryRMQ;
    delete[] summary;
}

size_t HybridRMQ::rmq(size_t low, size_t high) const {
    // If there is only one element, return that element.
    if (numElems == 1) {
        return low;
    }

    // first block after the block low is in
    int firstBlock = low == 0 ? 0 : (low - 1) / blockSize + 1;
    // last block before the block high is in
    int lastBlock = (high + 1) / blockSize - 1;

    // Use summaryRMQ to find the block containing the minimum element for the middle blocks, if there are any.
    int summaryMinIdx = firstBlock <= lastBlock ? summaryRMQ->rmq(firstBlock, lastBlock) : -1;

    // Perform a linear scan on the left and right edges of the range.
    RMQEntry minElem = elems[low];
    size_t minIdx = low;

    for (size_t i = low; i <= min(firstBlock * blockSize - 1, high); i++) {
        if (elems[i] < minElem) {
            minElem = elems[i];
            minIdx = i;
        }
    }

    for (int i = high; i >= max((int)low, (lastBlock + 1) * (int)blockSize); i--) {
        if (elems[i] < minElem) {
            minElem = elems[i];
            minIdx = i;
        }
    }

    if (summaryMinIdx != -1 && summary[summaryMinIdx] < minElem) {
        // Do a naive scan across the block corresponding to the minimum block to find the position of the minimum element.
        // Overall time complexity remains the same.

        for (size_t i = summaryMinIdx * blockSize; i < min(summaryMinIdx * blockSize + blockSize, numElems); i++) {
            if (elems[i] == summary[summaryMinIdx]) {
                minIdx = i;
                break;
            }
        }
    }

    return minIdx;
}


/* Feel free to add additional test cases here. These are basic checks for correctness
 * that do not account for all possible use cases.
 */
#include "SegmentTreeRMQ.h"
#include <random>
#include <algorithm>

PROVIDED_TEST("Works with a single-element array.") {
    vector<RMQEntry> array = { RMQEntry(137) };
    
    HybridRMQ rmq(array.data(), array.size());
    
    EXPECT_EQUAL(rmq.rmq(0, 0), 0);
}

PROVIDED_TEST("Works with a four-element array.") {
    vector<RMQEntry> array = { RMQEntry(1), RMQEntry(0), RMQEntry(2), RMQEntry(3) };
    
    HybridRMQ rmq(array.data(), array.size());
    
    /* Length one. */
    EXPECT_EQUAL(rmq.rmq(0, 0), 0);
    EXPECT_EQUAL(rmq.rmq(1, 1), 1);
    EXPECT_EQUAL(rmq.rmq(2, 2), 2);
    EXPECT_EQUAL(rmq.rmq(3, 3), 3);
    
    /* Length two. */
    EXPECT_EQUAL(rmq.rmq(0, 1), 1);
    EXPECT_EQUAL(rmq.rmq(1, 2), 1);
    EXPECT_EQUAL(rmq.rmq(2, 3), 2);
    
    /* Length three. */
    EXPECT_EQUAL(rmq.rmq(0, 2), 1);
    EXPECT_EQUAL(rmq.rmq(1, 3), 1);
    
    /* Length four. */
    EXPECT_EQUAL(rmq.rmq(0, 3), 1);
}

PROVIDED_TEST("Works with 1,000-element array.") {
    mt19937 generator(137); // Consistent random values
    
    /* Get a permutation of 0 ... 999. */
    vector<RMQEntry> array(1000);
    for (size_t i = 0; i < array.size(); i++) {
        array[i] = RMQEntry(i);
    }
    shuffle(array.begin(), array.end(), generator);
    
    /* Build an RMQ structure and a reference RMQ structure. */
    HybridRMQ rmq(array.data(), array.size());
    SegmentTreeRMQ ref(array.data(), array.size());
    
    /* Do all possible RMQs. */
    for (size_t i = 0; i < array.size(); i++) {
        for (size_t j = i; j < array.size(); j++) {
            EXPECT_EQUAL(rmq.rmq(i, j), ref.rmq(i, j));
        }
    }
}