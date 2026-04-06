#include "CANFrame.hpp"
#include <cstring>
#include <cstdio>

CANFrame CANFrame::buildStandard(uint16_t id, const uint8_t* data, uint8_t length) {
    CANFrame frame;

    // Standard ID is 11 bits max
    if (id > 0x7FF || length > MAX_DATA_LEN) {
        frame.valid = false;
        return frame;
    }

    frame.id        = id;
    frame.length    = length;
    frame.idType    = CANIDType::STANDARD;
    frame.frameType = CANFrameType::DATA;
    frame.valid     = true;

    if (data && length > 0)
        std::memcpy(frame.data.data(), data, length);

    return frame;
}

CANFrame CANFrame::buildExtended(uint32_t id, const uint8_t* data, uint8_t length) {
    CANFrame frame;

    // Extended ID is 29 bits max
    if (id > 0x1FFFFFFF || length > MAX_DATA_LEN) {
        frame.valid = false;
        return frame;
    }

    frame.id        = id;
    frame.length    = length;
    frame.idType    = CANIDType::EXTENDED;
    frame.frameType = CANFrameType::DATA;
    frame.valid     = true;

    if (data && length > 0)
        std::memcpy(frame.data.data(), data, length);

    return frame;
}

CANFrame CANFrame::buildRemote(uint16_t id, uint8_t length) {
    CANFrame frame;

    if (id > 0x7FF || length > MAX_DATA_LEN) {
        frame.valid = false;
        return frame;
    }

    frame.id        = id;
    frame.length    = length;
    frame.idType    = CANIDType::STANDARD;
    frame.frameType = CANFrameType::REMOTE;
    frame.valid     = true;

    return frame;
}

uint32_t CANFrame::encode(uint8_t* bitBuffer, uint32_t bufferSize) const {
    if (!valid || !bitBuffer) return 0;

    uint32_t pos = 0;

    // Helper lambda — push one bit into buffer
    auto pushBit = [&](uint8_t bit) {
        if (pos < bufferSize)
            bitBuffer[pos++] = bit;
    };

    // SOF — start of frame (dominant, 0)
    pushBit(0);

    if (idType == CANIDType::STANDARD) {
        // 11-bit ID, MSB first
        for (int i = 10; i >= 0; i--)
            pushBit((id >> i) & 1);

        // RTR bit
        pushBit(frameType == CANFrameType::REMOTE ? 1 : 0);

        // IDE bit — 0 for standard
        pushBit(0);

        // r0 — reserved
        pushBit(0);

    } else {
        // Extended frame — 11-bit base ID
        for (int i = 10; i >= 0; i--)
            pushBit((id >> 18 >> i) & 1);

        // SRR — substitute remote request (recessive, 1)
        pushBit(1);

        // IDE — 1 for extended
        pushBit(1);

        // 18-bit extension ID
        for (int i = 17; i >= 0; i--)
            pushBit((id >> i) & 1);

        // RTR bit
        pushBit(frameType == CANFrameType::REMOTE ? 1 : 0);

        // r1, r0 — reserved
        pushBit(0);
        pushBit(0);
    }

    // DLC — 4-bit data length code
    for (int i = 3; i >= 0; i--)
        pushBit((length >> i) & 1);

    // Data bytes
    for (uint8_t b = 0; b < length; b++)
        for (int i = 7; i >= 0; i--)
            pushBit((data[b] >> i) & 1);

    return pos;
}

void CANFrame::print() const {
    printf("CANFrame [%s] ID=0x%03X DLC=%d Data:",
        idType == CANIDType::STANDARD ? "STD" : "EXT",
        id, length);
    for (uint8_t i = 0; i < length; i++)
        printf(" %02X", data[i]);
    printf("\n");
}