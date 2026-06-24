// drone_link.h — binary UART protocol for HW11
#ifndef DRONE_LINK_H
#define DRONE_LINK_H
#include <cstdint>
#include <cstring>

namespace dlink {

constexpr uint8_t MAGIC0 = 0xA5;
constexpr uint8_t MAGIC1 = 0x5A;

enum PacketType : uint8_t {
    PKT_TELEMETRY = 0x01,
    PKT_TARGET    = 0x02,
    PKT_AMMO      = 0x03,
    PKT_RESULT    = 0x04,
    PKT_CONTROL   = 0x05,
};

#pragma pack(push, 1)

struct Telemetry {
    uint32_t t_ms;
    float x, y;
    float z;
    float vx, vy;
    float speed;
    float dir;
    uint8_t state;
};

struct TargetPos {
    uint8_t id;
    float x, y;
};

struct AmmoCfg {
    char    name[16];
    float   mass;
    float   drag;
    float   lift;
    float   hitRadius;
    uint8_t nTargets;
};

struct Result {
    uint8_t  hit;
    uint8_t  targetId;
    float    miss_m;
    uint32_t drop_t_ms;
};

struct Control {
    float accel;
    float turnRate;
};

#pragma pack(pop)

inline uint16_t crc16(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (int b = 0; b < 8; ++b)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
    return crc;
}

inline size_t encode(uint8_t type, const void* payload, uint8_t payloadLen, uint8_t* out) {
    out[0] = MAGIC0;
    out[1] = MAGIC1;
    out[2] = type;
    out[3] = payloadLen;
    if (payloadLen && payload) std::memcpy(out + 4, payload, payloadLen);
    uint16_t c = crc16(out + 2, (size_t)payloadLen + 2);
    out[4 + payloadLen]     = (uint8_t)(c & 0xFF);
    out[4 + payloadLen + 1] = (uint8_t)(c >> 8);
    return (size_t)payloadLen + 6;
}

struct Parser {
    enum { S_M0, S_M1, S_TYPE, S_LEN, S_PAYLOAD, S_CRC0, S_CRC1 } st = S_M0;
    uint8_t  type = 0, len = 0, idx = 0;
    uint8_t  buf[260];
    uint16_t crc_rx = 0;

    bool feed(uint8_t byte, uint8_t& outType, uint8_t* outPayload, uint8_t& outLen) {
        switch (st) {
        case S_M0:    if (byte == MAGIC0) st = S_M1; break;
        case S_M1:    st = (byte == MAGIC1) ? S_TYPE : S_M0; break;
        case S_TYPE:  type = byte; st = S_LEN; break;
        case S_LEN:   len = byte; idx = 0; st = len ? S_PAYLOAD : S_CRC0; break;
        case S_PAYLOAD:
            buf[idx++] = byte;
            if (idx >= len) st = S_CRC0;
            break;
        case S_CRC0: crc_rx = byte; st = S_CRC1; break;
        case S_CRC1: {
            crc_rx |= (uint16_t)byte << 8;
            st = S_M0;
            uint8_t tmp[262];
            tmp[0] = type; tmp[1] = len;
            std::memcpy(tmp + 2, buf, len);
            if (crc16(tmp, (size_t)len + 2) == crc_rx) {
                outType = type; outLen = len;
                std::memcpy(outPayload, buf, len);
                return true;
            }
            break;
        }
        }
        return false;
    }
};

} // namespace dlink
#endif // DRONE_LINK_H
