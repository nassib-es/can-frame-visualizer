#pragma once
#include <cstdint>
#include <array>

// CAN frame types
enum class CANFrameType {
    DATA,       // standard data frame
    REMOTE,     // remote transmission request (RTR)
    ERROR       // error frame
};

// CAN ID type
enum class CANIDType {
    STANDARD,   // 11-bit ID
    EXTENDED    // 29-bit ID
};

class CANFrame {
public:
    static constexpr uint8_t MAX_DATA_LEN = 8;

    CANFrame() = default;

    // Build a standard (11-bit ID) data frame
    static CANFrame buildStandard(uint16_t id, 
                                   const uint8_t* data, 
                                   uint8_t length);

    // Build an extended (29-bit ID) data frame
    static CANFrame buildExtended(uint32_t id, 
                                   const uint8_t* data, 
                                   uint8_t length);

    // Build a remote frame (no data, just a request)
    static CANFrame buildRemote(uint16_t id, uint8_t length);

    // Getters
    uint32_t getId()         const { return id; }
    uint8_t  getLength()     const { return length; }
    CANIDType getIDType()    const { return idType; }
    CANFrameType getType()   const { return frameType; }
    bool     isValid()       const { return valid; }

    const std::array<uint8_t, MAX_DATA_LEN>& getData() const { return data; }

    // Encode frame into raw bits (before bit stuffing)
    // Returns number of bits written
    uint32_t encode(uint8_t* bitBuffer, uint32_t bufferSize) const;

    // Decode raw bits into a frame
    static CANFrame decode(const uint8_t* bitBuffer, uint32_t numBits);

    // Print frame info (for debugging)
    void print() const;

private:
    uint32_t  id        = 0;
    uint8_t   length    = 0;
    CANIDType idType    = CANIDType::STANDARD;
    CANFrameType frameType = CANFrameType::DATA;
    bool      valid     = false;

    std::array<uint8_t, MAX_DATA_LEN> data = {};
};