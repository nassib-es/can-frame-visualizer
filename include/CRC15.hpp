#pragma once
#include <cstdint>

class CRC15 {
public:
    // CAN CRC-15 polynomial: 0x4599
    static constexpr uint16_t POLYNOMIAL = 0x4599;

    // Calculate CRC-15 over a bit stream
    // bits: array where each element is one bit (0 or 1)
    // length: number of bits
    static uint16_t calculate(const uint8_t* bits, uint32_t length);

    // Verify CRC — returns true if valid
    static bool verify(const uint8_t* bits, uint32_t length, uint16_t crc);
};