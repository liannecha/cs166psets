#include "FischerHeunRMQ.h"
#include "SimpleTest/SimpleTest.h"
using namespace std;

FischerHeunRMQ::FischerHeunRMQ(const RMQEntry* elems, size_t numElems) {
  /* TODO: Delete this line, the lines after this one, and implement
   * this function.
   */
  (void) elems;
  (void) numElems;
}

FischerHeunRMQ::~FischerHeunRMQ() {
  /* TODO: Delete this comment and implement this function. */
}

size_t FischerHeunRMQ::rmq(size_t low, size_t high) const {
  /* TODO: Delete this line, the lines after this one, and implement
   * this function.
   */
  (void) low;
  (void) high;
  return 0;
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
