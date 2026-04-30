/******************************************************************************
 * File: SixBitIntArray.h
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * Implementation of the SixBitIntArray type.
 */ 

#include "SixBitIntArray.h"
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <iostream>
using namespace std;

/* Default constructor creates a new, empty array. */
SixBitIntArray::SixBitIntArray() {
    data = nullptr;
    numElems = 0;
}

/* Main constructor makes enough space for everything, padding the size up to
 * a multiply of 64 bits.
 */
SixBitIntArray::SixBitIntArray(std::uint64_t numElems) {
    /* Number of bytes needed is numElems * 6 / 8. But, we have to round up,
     * so we conservatively always add one more byte.
     */
    uint64_t numBytes = (numElems * 6) / 8 + 1;
    data = operator new(numBytes);
    memset(data, 0, numBytes);
    
    this->numElems = numElems;
}

/* Cleans up memory. */
SixBitIntArray::~SixBitIntArray() {
    operator delete(data);
}

/* Copy constructor: Just clone the bits. */
SixBitIntArray::SixBitIntArray(const SixBitIntArray& rhs) {
    /* Don't clone a null pointer. */
    if (rhs.data) {
        uint64_t numBytes = (rhs.numElems * 6) / 8 + 1;

        /* Copy the raw bytes over into a new array. */
        data = operator new(numBytes);
        memcpy(data, rhs.data, numBytes);
    } else {
        data = nullptr;
    }
    
    numElems = rhs.numElems;
}

/* Move constructor: Steal things. */
SixBitIntArray::SixBitIntArray(SixBitIntArray&& rhs) {
    data = rhs.data;
    numElems = rhs.numElems;
    
    rhs.data = nullptr;
    rhs.numElems = 0;
}

/* Initialize-and-swap. */
SixBitIntArray& SixBitIntArray::operator= (SixBitIntArray rhs) {
    swap(data, rhs.data);
    swap(numElems, rhs.numElems);
    return *this;
}

/* Selection operator in the const case is a pure read. */
uint8_t SixBitIntArray::operator[] (uint64_t index) const {
    /* Bounds-check. */
    if (index >= numElems) {
        throw invalid_argument("SixBitIntArray::operator[]: Index is out of bounds.");
    }
    
    auto* bytes = static_cast<const std::uint8_t*>(data);
    uint64_t bitIndex  = index * 6;
    uint64_t byteIndex = bitIndex / 8;
    
    /* Different cases based on the mod-4 remainder. */
    if (index % 4 == 0) {
        /* Pull lower 6 bits from the appropriate byte. */
        return bytes[byteIndex] & 0b00111111;
    } else if (index % 4 == 1) {
        /* Pull upper 2 bits from the byte, and lower 4 from next byte. */
        return (bytes[byteIndex] >> 6) + (((bytes[byteIndex + 1]) & 0b00001111) << 2);
    } else if (index % 4 == 2) {
        /* Pull upper 4 bits from the byte, and lower 2 from the next byte. */
        return (bytes[byteIndex] >> 4) + (((bytes[byteIndex + 1]) & 0b00000011) << 4);
    } else {
        /* Pull upper six bits of the number. */
        return bytes[byteIndex] >> 2;
    }
}
    
/* Selection operator in the non-const case returns a proxy that does the
 * actual reads and writes.
 */
SixBitIntArray::IntProxy SixBitIntArray::operator[] (std::uint64_t index) {
    /* Bounds-check. */
    if (index >= numElems) {
        throw invalid_argument("SixBitIntArray::operator[]: Index is out of bounds.");
    }
    
    return IntProxy{ this, index };
}
    
uint64_t SixBitIntArray::size() const {
    return numElems;
}

/* Int proxy: A const read just forwards back to the main class accessor. */
SixBitIntArray::IntProxy::operator std::uint8_t() const {
    return const_cast<const SixBitIntArray &>(*parent)[index];
}

/* Int proxy: If you write to the type, the data gets sent back to the array. */
SixBitIntArray::IntProxy& SixBitIntArray::IntProxy::operator= (uint8_t value) {
    if (value > 0b111111) {
        throw invalid_argument("SixBitIntArray::operator[]: Writing a value with more than 6 bits.");
    }
    
    
    auto* bytes = static_cast<std::uint8_t*>(parent->data);
    uint64_t bitIndex  = index * 6;
    uint64_t byteIndex = bitIndex / 8;

    /* Use mod-4 remainder to determine what to do. */
    if (index % 4 == 0) {
        /* Write to lower 6 bits. */
        uint8_t clearMask = 0b11000000;
        uint8_t writeMask = value;
        bytes[byteIndex] = (bytes[byteIndex] & clearMask) | writeMask;
    } else if (index % 4 == 1) {
        /* Write to upper 2 bits and lower 4 bits. */
        uint8_t clearMask = 0b00111111;
        uint8_t writeMask = value << 6;
        bytes[byteIndex] = (bytes[byteIndex] & clearMask) | writeMask;
        
        clearMask = 0b11110000;
        writeMask = value >> 2;
        bytes[byteIndex + 1] = (bytes[byteIndex + 1] & clearMask) | writeMask;
    } else if (index % 4 == 2) {
        /* Write to upper 4 bits and lower 2 bits. */
        uint8_t clearMask = 0b00001111;
        uint8_t writeMask = value << 4;
        bytes[byteIndex] = (bytes[byteIndex] & clearMask) | writeMask;
        
        clearMask = 0b11111100;
        writeMask = (value >> 4) & 0b11;
        bytes[byteIndex + 1] = (bytes[byteIndex + 1] & clearMask) | writeMask;
    } else {
        /* Write to upper 6 bits. */
        uint8_t clearMask = 0b00000011;
        uint8_t writeMask = value << 2;
        bytes[byteIndex] = (bytes[byteIndex] & clearMask) | writeMask;
    } 
    return *this;
}

