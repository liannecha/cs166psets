#include "SparseTableRMQ.h"
#include "SimpleTest/SimpleTest.h"
using namespace std;

SparseTableRMQ::SparseTableRMQ(const RMQEntry* elems, size_t numElems) : numElems(numElems), elems(elems) {
    table = vector<vector<size_t>>(numElems);
    logs = vector<size_t>(numElems+1);

    // build logs table, logs[x] = floor(lg(x)). O(n) iterations, O(1) time per iter
    for (size_t i = 2; i <= numElems; i++) {
        logs[i] = logs[i/2] + 1;
    }

    // build sparse table
    // base case: lengths of 1.
    for (size_t i = 0; i < numElems; i++) {
        table[i].push_back(i);
    }

    // k is log(len)
    for (size_t k = 1; (size_t)(1 << k) <= numElems; k += 1) {
        size_t len = 1 << k;
        // i is the left index of the thing we are summarizing
        for (size_t i = 0; i + len - 1 < numElems; i++) {
            // dp step
            size_t half = 1 << (k - 1);
            size_t leftMin = table[i][k - 1];
            size_t rightMin = table[i + half][k - 1];

            size_t myMin = elems[leftMin] < elems[rightMin] ? leftMin : rightMin;

            table[i].push_back(myMin);
        }
    }
}

SparseTableRMQ::~SparseTableRMQ() {

}

size_t SparseTableRMQ::rmq(size_t low, size_t high) const {
    // find the k
    size_t k = logs[high - low + 1];
    size_t blockSize = 1 << k;
    size_t leftMin = table[low][k];
    size_t rightMin = table[high - blockSize + 1][k];

    return elems[leftMin] < elems[rightMin] ? leftMin : rightMin;
}

/* Feel free to add additional test cases here. These are basic checks for correctness
 * that do not account for all possible use cases.
 */
#include "SegmentTreeRMQ.h"
#include <random>
#include <algorithm>

PROVIDED_TEST("Works with a single-element array.") {
    vector<RMQEntry> array = { RMQEntry(137) };
    
    SparseTableRMQ rmq(array.data(), array.size());
    
    EXPECT_EQUAL(rmq.rmq(0, 0), 0);
}

PROVIDED_TEST("Works with a four-element array.") {
    vector<RMQEntry> array = { RMQEntry(1), RMQEntry(0), RMQEntry(2), RMQEntry(3) };
    
    SparseTableRMQ rmq(array.data(), array.size());
    
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
    SparseTableRMQ rmq(array.data(), array.size());
    SegmentTreeRMQ ref(array.data(), array.size());
    
    /* Do all possible RMQs. */
    for (size_t i = 0; i < array.size(); i++) {
        for (size_t j = i; j < array.size(); j++) {
            EXPECT_EQUAL(rmq.rmq(i, j), ref.rmq(i, j));
        }
    }
}
