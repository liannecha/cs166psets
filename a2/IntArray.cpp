/******************************************************************************
 * File: IntArray.h
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * Implementation of the IntArray type.
 */ 

#include "IntArray.h"
#include "Utility.h"
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <iostream>
using namespace std;

/* Default constructor creates a new, empty array. */
IntArray::IntArray() {
    data = nullptr;
    elemSize = 0;
    numElems = 0;
}

/* Main constructor makes enough space for everything, padding the size up to
 * a multiply of 64 bits.
 */
IntArray::IntArray(BitCount elemSize, std::uint64_t numElems) {
    data = allocate(divideAndRoundUp(elemSize.bitCount() * numElems, 8));
    this->elemSize = elemSize.bitCount();
    this->numElems = numElems;
}

/* Cleans up memory. */
IntArray::~IntArray() {
    if (data) deallocate(data);
}

/* Copy constructor: Just clone the bits. */
IntArray::IntArray(const IntArray& rhs) {
    /* Don't clone a null pointer. */
    if (rhs.data) {
        uint64_t numBytes = divideAndRoundUp(rhs.elemSize * rhs.numElems, 8);

        /* Copy the raw bytes over into a new array. */
        data = allocate(numBytes);
        memcpy(data, rhs.data, numBytes);
    } else {
        data = nullptr;
    }
    
    elemSize = rhs.elemSize;
    numElems = rhs.numElems;
}

/* Move constructor: Steal things. */
IntArray::IntArray(IntArray&& rhs) {
    data = rhs.data;
    elemSize = rhs.elemSize;
    numElems = rhs.numElems;
    
    rhs.data = nullptr;
    rhs.elemSize = 0;
    rhs.numElems = 0;
}

/* Initialize-and-swap. */
IntArray& IntArray::operator= (IntArray rhs) {
    swap(data, rhs.data);
    swap(elemSize, rhs.elemSize);
    swap(numElems, rhs.numElems);
    return *this;
}

/* Selection operator in the const case is a pure read. */
uint64_t IntArray::operator[] (uint64_t index) const {
    /* Bounds-check. */
    if (index >= numElems) {
        throw invalid_argument("IntArray::operator[]: Index is out of bounds.");
    }
    
    return integerAt(data, index * elemSize, elemSize);
}
    
/* Selection operator in the non-const case returns a proxy that does the
 * actual reads and writes.
 */
IntArray::IntProxy IntArray::operator[] (std::uint64_t index) {
    /* Bounds-check. */
    if (index >= numElems) {
        throw invalid_argument("IntArray::operator[]: Index is out of bounds.");
    }
    
    return IntProxy{ this, index };
}
    
uint64_t IntArray::size() const {
    return numElems;
}

BitCount IntArray::bitsPerElem() const {
    return BitCount(elemSize);
}

/* Int proxy: If you pretend it's an integer, you get back the underlying bits. */
IntArray::IntProxy::operator std::uint64_t() const {
    return integerAt(parent->data, index * parent->elemSize, parent->elemSize);
}

/* Int proxy: If you write to the type, the data gets sent back to the array. */
IntArray::IntProxy& IntArray::IntProxy::operator= (uint64_t value) {
    if (value >= (uint64_t(1) << parent->elemSize)) {
        throw invalid_argument("IntArray::operator[]: Writing a value with more bits than the bit width.");
    }
    
    writeIntegerAt(parent->data, index * parent->elemSize, parent->elemSize, value);
    return *this;
}

