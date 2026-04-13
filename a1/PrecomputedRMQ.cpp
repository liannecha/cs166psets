#include "PrecomputedRMQ.h"
#include "SimpleTest/SimpleTest.h"
using namespace std;

PrecomputedRMQ::PrecomputedRMQ(const RMQEntry* elems, size_t numElems) : numElems(numElems) {
    // init table
    table = new size_t*[numElems];

    // to save 1/2 space we only allocate what we need
    for (size_t i = 0; i < numElems; i++) {
        table[i] = new size_t[numElems - i];
    }

    // i is left index of query
    for (size_t i = 0; i < numElems; i++) {
        // j is right index of query
        for (size_t j = i; j < numElems; j++) {
            RMQEntry minElem = elems[i];
            size_t minIndex = i;

            // linear scan for smallest elem
            for (size_t k = i+1; k <= j; k++) {
                if (elems[k] < minElem) {
                    minElem = elems[k];
                    minIndex = k;
                }
            }

            table[i][j-i] = minIndex;
        }
    }
}

PrecomputedRMQ::~PrecomputedRMQ() {
  for (size_t i = 0; i < numElems; i++) {
    delete[] table[i];
  }

  delete[] table;
}

size_t PrecomputedRMQ::rmq(size_t low, size_t high) const {
  return table[low][high - low];
}


/* Feel free to add additional test cases here. These are basic checks for correctness
 * that do not account for all possible use cases.
 */
#include "SegmentTreeRMQ.h"
#include <random>
#include <algorithm>

PROVIDED_TEST("Works with a single-element array.") {
    vector<RMQEntry> array = { RMQEntry(137) };
    
    PrecomputedRMQ rmq(array.data(), array.size());
    
    EXPECT_EQUAL(rmq.rmq(0, 0), 0);
}

PROVIDED_TEST("Works with a four-element array.") {
    vector<RMQEntry> array = { RMQEntry(1), RMQEntry(0), RMQEntry(2), RMQEntry(3) };
    
    PrecomputedRMQ rmq(array.data(), array.size());
    
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
    PrecomputedRMQ rmq(array.data(), array.size());
    SegmentTreeRMQ ref(array.data(), array.size());
    
    /* Do all possible RMQs. */
    for (size_t i = 0; i < array.size(); i++) {
        for (size_t j = i; j < array.size(); j++) {
            EXPECT_EQUAL(rmq.rmq(i, j), ref.rmq(i, j));
        }
    }
}
