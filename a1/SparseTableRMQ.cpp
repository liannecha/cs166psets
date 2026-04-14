#include "SparseTableRMQ.h"
#include "SimpleTest/SimpleTest.h"
using namespace std;

SparseTableRMQ::SparseTableRMQ(const RMQEntry* elems, size_t numElems) {
    this->elems = elems;
    if (numElems == 0) return;

    // Build log table: [0, 0, 1, 1, 2, 2, ...]
    log_table.resize(numElems + 1);
    log_table[1] = 0;
    for (size_t i = 2; i <= numElems; i++) {
        log_table[i] = log_table[i / 2] + 1;
    }

    // Build sparse table (dim=max_log+1 x numElems)
    size_t max_log = log_table[numElems];
    sparse_table.resize(max_log + 1);
    
    // Resize each row of the sparse table
    for (size_t k = 0; k <= max_log; k++) {
        size_t interval_length = 1 << k;
        size_t num_intervals = numElems - interval_length + 1;
        sparse_table[k].resize(num_intervals);
    }

    // Fill in first row of sparse table (intervals of length 1)
    for (size_t i = 0; i < numElems; i++) {
        sparse_table[0][i] = i;
    }

    // Fill in the rest of the sparse table
    for (size_t k = 1; k <= max_log; k++) {
        size_t half_interval_length = 1 << (k - 1);
        size_t num_intervals = sparse_table[k].size();

        for (size_t i = 0; i < num_intervals; i++) {
            // Get the indices of the two halves of the interval
            size_t left_index = sparse_table[k - 1][i];
            size_t right_index = sparse_table[k - 1][i + half_interval_length];

            // Compare the two halves and store the index of the minimum
            if (elems[left_index] < elems[right_index]) {
                sparse_table[k][i] = left_index;
            } else {
                sparse_table[k][i] = right_index;
            }
        }
    }
}

SparseTableRMQ::~SparseTableRMQ() {
}

size_t SparseTableRMQ::rmq(size_t low, size_t high) const {
    // Compute query range length and corresponding power of 2
    size_t query_range = high - low + 1;
    size_t k = log_table[query_range];
    size_t interval_length = 1 << k;

    // Get the indices of the intervals covering the query range
    size_t left_index = sparse_table[k][low];
    size_t right_index = sparse_table[k][high - interval_length + 1];

    // Return the index of the minimum value in the query range
    if (elems[left_index] < elems[right_index]) {
        return left_index;
    } else {
        return right_index;
    }
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
