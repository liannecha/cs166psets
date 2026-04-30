/******************************************************************************
 * File: SixBitIntArray.h
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * Type representing a fixed-size array of integers that are all exactly six
 * bits each. This behaves almost identically to IntArray, except that, as
 * the name suggests, it's optimized for integers that are six bits each.
 */   

#pragma once

#include <cstdint>
#include <stdexcept>

/* Type representing a fixed-sized array of integers with six bits each. */
class SixBitIntArray {
public:
    /* Constructs a new, empty SixBitIntArray. */
    SixBitIntArray();
    
    /* Constructs a new SixBitIntArray of the given size. All elements are
     * initialized to 0.
     */
    SixBitIntArray(std::uint64_t numElems);
    
    /* Copy and move operators. */
    SixBitIntArray(const SixBitIntArray&);
    SixBitIntArray(SixBitIntArray&&);
    SixBitIntArray& operator= (SixBitIntArray);
    
    /* Cleans up memory used by this type. */
    ~SixBitIntArray();
    
    /* Integer reading. */
    std::uint8_t operator[] (std::uint64_t index) const;
    
    /* Integer writing. */
    class IntProxy;
    IntProxy operator[] (std::uint64_t index);
    
    /* How many elements are in the array. */
    std::uint64_t size() const;

private:
    void*         data;     // Raw element storage
    std::uint64_t numElems; // Number of elements
};

/* Proxy type that's used to read and write to an IntArray. Reading from this
 * type fetches the bits from the underlying array. Writing to this type
 * validates the size of the number being written, then writes the bits back
 * to the underlying array.
 */
class SixBitIntArray::IntProxy {
public:
    /* Implicit conversion to a uint8_t so that this can act like a number. */
    operator std::uint8_t() const;
    
    /* Write operation. */
    IntProxy& operator= (std::uint8_t value);

private:
    IntProxy(SixBitIntArray* parent, std::uint64_t index) : parent(parent), index(index) {
    
    }
    
    friend class SixBitIntArray;

    SixBitIntArray* parent;
    std::uint64_t index;
};
