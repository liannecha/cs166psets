/******************************************************************************
 * File: CardinalityEstimator.h
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * Some C++ shenanigans to make it possible to map from names of cardinality
 * estimator types to the underlying types.
 */

#pragma once
#include "../Hashes.h"
#include <string>
#include <typeinfo>
#include <functional>
#include <stdexcept>
#include <memory>
#include <cstdlib>
#include <cxxabi.h> // Nonstandard, but supported on g++ and clang.
#include <cstdint>

/* Base type for all cardinality estimators. Not intended to be
 * used directly by clients.
 */
class CardinalityEstimatorImpl {
public:
    virtual ~CardinalityEstimatorImpl() = default;
    virtual void see(std::uint64_t) = 0;
    virtual std::uint64_t estimate() const = 0;
};

/* Type representing an estimator; this should be used by clients. */
using Estimator = std::shared_ptr<CardinalityEstimatorImpl>;

/* A "registry" of all the cardinality estimator types. */
namespace Registry {
    /* Type that constructs an estimator given constructor args. */
    using Constructor = std::function<Estimator (HashFunction, std::uint64_t)>;
    
    /* Adds an estimator to the registry; not meant to be used by clients. */
    void addEstimator(const std::string& name, Constructor constructor);
    
    /* Returns a constructor for an estimator with the given name. */
    Constructor estimatorNamed(const std::string& name);
}

/* Type using the Curiously Recurring Template Pattern (CRTP) to automagically
 * register the derived type. Use this by writing something like
 *
 *    class MyEstimator: public CardinalityEstimator<MyEstimator> {
 *       ...
 *    };
 */
template <typename T> class CardinalityEstimator: public CardinalityEstimatorImpl {
public:
    /* Make use of the _initializer variable so that it is actually instantiated. */
    ~CardinalityEstimator() {
        _initializer++;
    }

private:
    /* Value that is static-initialized at program construction with something that
     * constructs the registry.
     */
    static int _initializer;
    static int registerType() {
        /* std::type_info does not guarantee that .name() will be at all human-readable.
         * Use this g++/clang-specific logic to "demangle" the name back into a human-readable
         * format.
         */
        int statusCode;
        auto* realName = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &statusCode);
        if (statusCode != 0) throw std::runtime_error("Internal error: Couldn't demangle name?");

        /* Store the constructor. */
        Registry::addEstimator(std::string(realName), [](HashFunction hashFn, std::uint64_t precision) {
            return std::make_shared<T>(hashFn, precision);
        });

        std::free(realName);
        return 137; // They said I could return anything, so I returned 137.
    }
};

template <typename T>
int CardinalityEstimator<T>::_initializer = CardinalityEstimator<T>::registerType();
