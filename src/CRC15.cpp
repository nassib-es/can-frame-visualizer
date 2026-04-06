#include "CRC15.hpp"

uint16_t CRC15::calculate(const uint8_t* bits, uint32_t length) {
    uint16_t crc = 0x0000;

    for (uint32_t i = 0; i < length; i++) {
        // XOR incoming bit with MSB of CRC
        uint16_t bit = bits[i] & 1;
        uint16_t msb = (crc >> 14) & 1;

        crc <<= 1;
        crc &= 0x7FFF;  // keep 15 bits only

        if (msb ^ bit)
            crc ^= POLYNOMIAL;
    }

    return crc;
}

bool CRC15::verify(const uint8_t* bits, uint32_t length, uint16_t crc) {
    return calculate(bits, length) == crc;
}