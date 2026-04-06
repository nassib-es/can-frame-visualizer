#include "BitStuffing.hpp"

uint32_t BitStuffing::stuff(const uint8_t* input, uint32_t inputLen,
                             uint8_t* output, uint32_t outputSize) {
    uint32_t outPos      = 0;
    uint8_t  consecutive = 1;
    uint8_t  lastBit     = input[0] & 1;

    if (outPos < outputSize)
        output[outPos++] = lastBit;

    for (uint32_t i = 1; i < inputLen; i++) {
        uint8_t bit = input[i] & 1;

        if (bit == lastBit) {
            consecutive++;
        } else {
            consecutive = 1;
        }

        if (outPos < outputSize)
            output[outPos++] = bit;

        lastBit = bit;

        // After 5 identical bits — insert opposite stuff bit
        if (consecutive == 5) {
            uint8_t stuffBit = !bit;
            if (outPos < outputSize)
                output[outPos++] = stuffBit;
            consecutive = 1;    // stuff bit counts as 1
            lastBit     = stuffBit;
        }
    }

    return outPos;
}

uint32_t BitStuffing::destuff(const uint8_t* input, uint32_t inputLen,
                               uint8_t* output, uint32_t outputSize) {
    uint32_t outPos      = 0;
    uint8_t  consecutive = 1;
    uint8_t  lastBit     = input[0] & 1;
    bool     expectStuff = false;

    if (outPos < outputSize)
        output[outPos++] = lastBit;

    for (uint32_t i = 1; i < inputLen; i++) {
        uint8_t bit = input[i] & 1;

        if (expectStuff) {
            if (bit == lastBit) {
                return 0;  // stuffing violation
            }
            expectStuff = false;
            consecutive = 1;
            lastBit     = bit;
            continue;
        }

        if (bit == lastBit) {
            consecutive++;
            if (consecutive == 5)
                expectStuff = true;
        } else {
            consecutive = 1;
        }

        if (outPos < outputSize)
            output[outPos++] = bit;

        lastBit = bit;
    }

    return outPos;
}

uint32_t BitStuffing::countStuffBits(const uint8_t* input, uint32_t inputLen) {
    uint32_t count       = 0;
    uint8_t  consecutive = 1;
    uint8_t  lastBit     = input[0];

    for (uint32_t i = 1; i < inputLen; i++) {
        uint8_t bit = input[i] & 1;

        if (bit == lastBit) {
            consecutive++;
            if (consecutive == 5) {
                count++;
                consecutive = 0;
            }
        } else {
            consecutive = 1;
        }

        lastBit = bit;
    }

    return count;
}