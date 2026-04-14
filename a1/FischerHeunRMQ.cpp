#include "FischerHeunRMQ.h"
#include "SimpleTest/SimpleTest.h"
#include "SparseTableRMQ.h"
using namespace std;

FischerHeunRMQ::FischerHeunRMQ(const RMQEntry* elems, size_t numElems) {
    // Copy the input array and number of elements.
    this->elems = elems;
    this->numElems = numElems;

    summaryRMQ = nullptr;
    summary = nullptr;

    // If the input array is empty, return.
    if (numElems == 0) {
        return;
    }

    // Compute block size.
    blockSize = max(1, (int)(log2(numElems) / 4));

    // Compute the number of blocks and allocate memory.
    numBlocks = (numElems - 1) / blockSize + 1;
    summary = new RMQEntry[(numElems-1)/blockSize + 1];
    minimum_indices.resize(numBlocks);
    block_types.resize(numBlocks);

    // Compute the minimum index and value for each block.
    for (size_t i = 0; i < numBlocks; i++) {
        size_t start = i * blockSize;
        size_t end = min(start + blockSize, numElems);
        size_t min_index = start;

        for (size_t j = start + 1; j < end; j++) {
            if (elems[j] < elems[min_index]) {
                min_index = j;
            }
        }

        summary[i] = elems[min_index];
        minimum_indices[i] = min_index;
    }

    // Build the sparse table for the summary array.
    summaryRMQ = new SparseTableRMQ(summary, numBlocks);

    // Compute the tree number and store in the type table.
    for (size_t i = 0; i < numBlocks; i++) {
        size_t start = i * blockSize;
        size_t len = min(blockSize, numElems - start);

        // Compute the Cartesian tree number for the block.
        size_t tree_number = 0;
        vector<size_t> stack;

        for (size_t j = 0; j < len; j++) {
            while (!stack.empty() && elems[start + stack.back()] > elems[start + j]) {
                stack.pop_back();
                tree_number = (tree_number << 1) | 1;
            }
            stack.push_back(j);
            tree_number <<= 1;
        }

        while (!stack.empty()) {
            stack.pop_back();
            tree_number <<= 1;
        }

        block_types[i] = tree_number;

        // If the tree number is not in the precomputed table, compute and store the RMQ solution for that tree number.
        if (precomputed_tree_numbers.find(tree_number) == precomputed_tree_numbers.end()) {
            vector<size_t> table(len * len);

            for (size_t l = 0; l < len; l++) {
                size_t min_index = l;
                table[l * len + l] = min_index;

                for (size_t r = l + 1; r < len; r++) {
                    if (elems[start + r] < elems[start + min_index]) {
                        min_index = r;
                    }
                    table[l * len + r] = min_index;
                }
            }
            precomputed_tree_numbers[tree_number] = table;
        }
    }
}

FischerHeunRMQ::~FischerHeunRMQ() {
    delete summaryRMQ;
    delete[] summary;
}

size_t FischerHeunRMQ::rmq(size_t low, size_t high) const {
    size_t low_block = low / blockSize;
    size_t high_block = high / blockSize;

    // If the range is within a single block, use the precomputed table.
    if (low_block == high_block) {
        size_t start = low_block * blockSize;
        size_t len = min(blockSize, numElems - start);
        size_t tree_number = block_types[low_block];

        size_t inner_block_low  = low - start;
        size_t inner_block_high = high - start;

        return start + precomputed_tree_numbers.at(tree_number)[inner_block_low * len + inner_block_high];
    }

    // Check the left and right edges of the range.
    size_t left_start = low_block * blockSize;
    size_t left_len = min(blockSize, numElems - left_start);
    size_t left_tree_number = block_types[low_block];

    size_t left_inner_block_index_low = low - left_start;
    size_t left_inner_block_index_high = left_len - 1;

    size_t solution = left_start + precomputed_tree_numbers.at(left_tree_number)[left_inner_block_index_low * left_len + left_inner_block_index_high];

    size_t right_start = high_block * blockSize;
    size_t right_len = min(blockSize, numElems - right_start);
    size_t right_tree_number = block_types[high_block];

    size_t right_inner_block_index_low = 0;
    size_t right_inner_block_index_high = high - right_start;

    size_t right_solution = right_start + precomputed_tree_numbers.at(right_tree_number)[right_inner_block_index_low * right_len + right_inner_block_index_high];

    if (elems[right_solution] < elems[solution]) {
        solution = right_solution;
    }

    // Check the middle blocks using the summary RMQ.
    if (low_block + 1 < high_block) {
        size_t middle_block = summaryRMQ->rmq(low_block + 1, high_block - 1);
        size_t middle_solution = minimum_indices[middle_block];

        if (elems[middle_solution] < elems[solution]) {
            solution = middle_solution;
        }
    }

    return solution;
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
