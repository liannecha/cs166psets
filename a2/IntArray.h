/******************************************************************************
 * File: IntArray.h
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * Type representing a fixed-size array of integers whose bit sizes are not
 * necessarily a nice power of two. Specifically, this type can handle arrays
 * of integers of 0 - 64 bits, inclusive.
 *
 * To create an IntArray, you will need to specify both the number of bits
 * per integer in the array and the number of integers. To make it harder to
 * get the arguments in the wrong order, you will need to specify the number
 * of bits using the BitCount type. For example:
 *
 *    // Array of 137 numbers, each of which is 29 bits long.
 *    IntArray arr(BitCount(29), 137);
 *
 * You can then use the selection operator [] to read and write individual
 * integers:
 *
 *    arr[106] = 2718;
 *    cout << arr[106] << endl; // Prints 2718
 *
 * The selection operator will return its result as a uint64_t, of which
 * only the appropriate number of bits will be used. Storing to an array
 * index will trigger a std::invalid_argument exception if you try to
 * write a number with more bits used than the stored bit size.
 *
 * Internally, everything here forwards to integerAt and writeIntegerAt,
 * which do all the heavy lifting.
 */   

#pragma once

#include <cstdint>
#include <stdexcept>

/* Type representing a number of bits. This type exists so that you cannot
 * easily mix up the number of bits and the number of items when constructing
 * an IntArray.
 */
class BitCount {
public:
    explicit BitCount(std::uint8_t numBits) : count(numBits) {
        if (numBits > 64) {
            throw std::invalid_argument("BitCount::BitCount(std::uint8_t): Bit count can't be more than 64.");
        }
    }
    
    std::uint8_t bitCount() const {
        return count;
    }

private:
    std::uint8_t count;
};

/* Type representing a fixed-sized array of integers with a nonstandard
 * number of bits.
 */
class IntArray {
public:
    /* Constructs a new, empty IntArray. */
    IntArray();
    
    /* Constructs a new IntArray where each integer has elemSize bits and there
     * are numElems total elements, all initially zero.
     */
    IntArray(BitCount elemSize, std::uint64_t numElems);
    
    /* Copy and move operators. */
    IntArray(const IntArray&);
    IntArray(IntArray&&);
    IntArray& operator= (IntArray);
    
    /* Cleans up memory used by this type. */
    ~IntArray();
    
    /* Integer reading. */
    std::uint64_t operator[] (std::uint64_t index) const;
    
    /* Integer writing. */
    class IntProxy;
    IntProxy operator[] (std::uint64_t index);
    
    /* How many elements are in the array. */
    std::uint64_t size() const;
    
    /* How many bits each element is. */
    BitCount bitsPerElem() const;

private:
    void*         data;     // Raw element storage
    std::uint8_t  elemSize; // Bits per element
    std::uint64_t numElems; // Number of elements
};

/* Proxy type that's used to read and write to an IntArray. Reading from this
 * type fetches the bits from the underlying array. Writing to this type
 * validates the size of the number being written, then writes the bits back
 * to the underlying array.
 */
class IntArray::IntProxy {
public:
    /* Implicit conversion to a uint64_t so that this can act like a number. */
    operator std::uint64_t() const;
    
    /* Write operation. */
    IntProxy& operator= (std::uint64_t value);

private:
    IntProxy(IntArray* parent, std::uint64_t index) : parent(parent), index(index) {
    
    }
    
    friend class IntArray;

    IntArray* parent;
    std::uint64_t index;
};
