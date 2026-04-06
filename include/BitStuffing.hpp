#pragma once
#include <cstdint>

class BitStuffing {
public:
    // After 5 consecutive identical bits, insert the opposite bit
    // input: raw bit stream
    // output: stuffed bit stream
    // returns: number of bits after stuffing
    static uint32_t stuff(const uint8_t* input, uint32_t inputLen,
                          uint8_t* output, uint32_t outputSize);

    // Remove stuffed bits from a received bit stream
    // returns: number of bits after destuffing
    // returns 0 if a stuffing violation is detected (error)
    static uint32_t destuff(const uint8_t* input, uint32_t inputLen,
                            uint8_t* output, uint32_t outputSize);

    // Count how many stuff bits were inserted
    static uint32_t countStuffBits(const uint8_t* input, uint32_t inputLen);
};