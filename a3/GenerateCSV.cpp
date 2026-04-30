/**
 * File: GenerateCSV.cpp
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * Program that runs the specified cardinality estimator on a range of
 * values of n, outputting the sample mean and variance of the estimates.
 */
#include "Utility/CardinalityEstimator.h"
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
using namespace std;

namespace {
    void printUsage() {
        cerr << "Usage: generate-csv type precision" << endl;
        cerr << "   type is the name of the type in question (e.g. NaiveHyperLogLog)" << endl;
        cerr << "   precision is the precision value p, which must be between 3 and 16, inclusive." << endl;
    }
    
    struct Statistics {
        uint64_t mean;
        uint64_t variance;
    };
    
    /* Returns mean and variance of the estimates from a group of estimators. */
    Statistics statisticsFor(const vector<Estimator>& estimators) {
        /* Get the estimates. */
        vector<uint64_t> estimates;
        for (const auto& estimator: estimators) {
            estimates.push_back(estimator->estimate());
        }
        
        /* Compute the variance. */
        double mean = accumulate(estimates.begin(), estimates.end(), 0.0) / estimates.size();
        double variance = 0.0;
        for (auto estimate: estimates) {
            variance += (double(estimate) - mean) * (double(estimate) - mean) / estimates.size();
        }
        
        return {
            uint64_t(mean), uint64_t(variance)
        };
    }
    
    /* Parameters controlling where we sample. */
    const uint64_t kHigh = 100000;
    const double   kMultiplier = 1.5;
    
    const uint64_t kNumEstimators = 5000;
}

int main(int argc, const char* argv[]) try {
    /* Validate arguments. */
    if (argc != 3) {
        printUsage();
        exit(-1);
    }
    
    /* Look up the estimator with the given name. */
    auto constructor = Registry::estimatorNamed(argv[1]);
    
    /* Get an offset value for the random seed so that we don't just
     * get the same values each time.
     */
    auto randomOffset = random_device()();
    
    /* Construct a wide array of estimators to "denoise" the result. */
    vector<Estimator> estimators;
    for (uint64_t i = 0; i < kNumEstimators; i++) {
        /* Use tabulation hashing with different seeds per estimator so
         * that each one has its own hash function.
         */
        estimators.push_back(constructor(tabulation(i + uint64_t(randomOffset)), stod(argv[2])));
    };
    
    cout << "Cardinality,Mean,Stdev" << '\n';
    
    /* Do a bunch of insertions, periodically querying for estimates. */
    uint64_t n = 0;
    while (true) {
        auto stats = statisticsFor(estimators);
        cout << n << ',' << stats.mean << ',' << static_cast<uint64_t>(sqrt(stats.variance)) << '\n';
        
        /* Grow exponentially, but always increment by one to avoid stalling
         * at the low end of the range.
         */
        uint64_t stopPoint = n * kMultiplier + 1;
        if (stopPoint > kHigh) break;
        
        /* Use sequential values for the see function; after hashing
         * this is about as good as picking a truly random collection
         * of items.
         */
        for (; n < stopPoint; n++) {
            for (auto& estimator: estimators) {
                estimator->see(n);
            }
        }
    }
    
    return 0;    
} catch (const exception& e) {
    cerr << "Error: " << e.what() << endl;
    return -1;
}
