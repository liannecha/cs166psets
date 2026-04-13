#include "FischerHeunRMQ.h"
#include "SimpleTest/SimpleTest.h"
#include <string.h>
#include <iostream>
using namespace std;

int FischerHeunRMQ::computeCartesianNumber(size_t low, size_t high) {
    
}

FischerHeunRMQ::FischerHeunRMQ(const RMQEntry* elems, size_t numElems) : elems(elems), numElems(numElems) {
    summaryRMQ = NULL;
    summary = NULL;

  blockSize = ceil(log2(numElems)) / 4;
  numRMQs = pow(4, blockSize);
  if (blockSize == 0) {
    rmqs = new PrecomputedRMQ*[1];
    rmqs[0] = new PrecomputedRMQ(elems, numElems);
    return;
  } else {
    rmqs = new PrecomputedRMQ*[numRMQs];
    memset(rmqs, 0, sizeof(PrecomputedRMQ*) * numRMQs);
  }

  summary = new RMQEntry[(numElems-1)/blockSize + 1];
  
  // iterate over blocks
  for (size_t b = 0; b < (numElems-1)/blockSize + 1; b++) {
    // iterate over entries within block
    RMQEntry minElem = elems[b*blockSize];

    for (size_t i = b*blockSize; i < min(b*blockSize + blockSize, numElems); i++) {
        if (elems[i] < minElem) {
            minElem = elems[i];
        }
    }

    summary[b] = minElem;
  }

  summaryRMQ = new SparseTableRMQ(summary, (numElems-1)/blockSize + 1);
}

FischerHeunRMQ::~FischerHeunRMQ() {
  delete summaryRMQ;
  delete[] summary;

  if (blockSize == 0) {
    delete rmqs[0];
  } else {
    for (size_t i = 0; i < numRMQs; i++) {
        delete rmqs[i];
    }
  }

  delete[] rmqs;
}

size_t FischerHeunRMQ::rmq(size_t low, size_t high) const {
    if (blockSize == 0) {
        return rmqs[0]->rmq(low, high);
    }

  // first block after the block low is in
  int firstBlock = low == 0 ? 0 : (low - 1) / blockSize + 1;
  // last block before the block high is in
  int lastBlock = (high + 1) / blockSize - 1;

  int summaryMinIdx = firstBlock <= lastBlock ? summaryRMQ->rmq(firstBlock, lastBlock) : -1;

  // query edges
  size_t startBlock = low/blockSize;
  size_t leftStart = low;
  size_t leftEnd = min(high, (startBlock + 1) * blockSize - 1);

  size_t minIdx = rmqs[startBlock]->rmq(leftStart % blockSize, leftEnd % blockSize);
  RMQEntry minElem = elems[minIdx];

  size_t highBlock = high/blockSize;
  size_t rightStart = max(high / blockSize * blockSize, low);
  size_t rightEnd = high;

  size_t rightIdx = rmqs[highBlock]->rmq(rightStart % blockSize, rightEnd % blockSize);

  if (elems[rightIdx] < minElem) {
    minElem = elems[rightIdx];
    minIdx = rightIdx;
  }

  if (summaryMinIdx != -1 && summary[summaryMinIdx] < minElem) {
    // do a naive scan across the block corresponding to the minimum block to find the position of the minimum element.
    // overall time complexity remains the same.

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
    
    FischerHeunRMQ rmq(array.data(), array.size());
    
    EXPECT_EQUAL(rmq.rmq(0, 0), 0);
}

PROVIDED_TEST("Works with a four-element array.") {
    vector<RMQEntry> array = { RMQEntry(1), RMQEntry(0), RMQEntry(2), RMQEntry(3) };
    
    FischerHeunRMQ rmq(array.data(), array.size());
    
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
    FischerHeunRMQ rmq(array.data(), array.size());
    SegmentTreeRMQ ref(array.data(), array.size());
    
    /* Do all possible RMQs. */
    for (size_t i = 0; i < array.size(); i++) {
        for (size_t j = i; j < array.size(); j++) {
            EXPECT_EQUAL(rmq.rmq(i, j), ref.rmq(i, j));
        }
    }
}
