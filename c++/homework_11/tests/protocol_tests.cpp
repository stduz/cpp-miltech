#include "drone_link.h"
#include <gtest/gtest.h>
#include <cstring>

using namespace dlink;

TEST(CRC16, Idempotent) {
    uint8_t data[] = {0x05, 0x08, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F};
    EXPECT_EQ(crc16(data, sizeof data), crc16(data, sizeof data));
}

TEST(CRC16, DifferentDataDifferentCRC) {
    uint8_t a[] = {0x01, 0x04};
    uint8_t b[] = {0x01, 0x05};
    EXPECT_NE(crc16(a, 2), crc16(b, 2));
}

TEST(Encode, FrameLayout) {
    Control c{0.5f, -0.3f};
    uint8_t out[64];
    size_t m = encode(PKT_CONTROL, &c, sizeof c, out);
    EXPECT_EQ(m, sizeof(Control) + 6u);
    EXPECT_EQ(out[0], MAGIC0);
    EXPECT_EQ(out[1], MAGIC1);
    EXPECT_EQ(out[2], PKT_CONTROL);
    EXPECT_EQ(out[3], (uint8_t)sizeof(Control));
}

TEST(Parser, RoundTrip) {
    Control c{0.75f, -0.5f};
    uint8_t out[64];
    size_t m = encode(PKT_CONTROL, &c, sizeof c, out);

    Parser p;
    uint8_t type, len;
    uint8_t payload[260];
    bool got = false;
    for (size_t i = 0; i < m; i++) {
        if (p.feed(out[i], type, payload, len)) { got = true; break; }
    }
    ASSERT_TRUE(got);
    EXPECT_EQ(type, PKT_CONTROL);
    EXPECT_EQ(len, (uint8_t)sizeof(Control));
    Control c2;
    std::memcpy(&c2, payload, sizeof c2);
    EXPECT_NEAR(c2.accel,    c.accel,    1e-5f);
    EXPECT_NEAR(c2.turnRate, c.turnRate, 1e-5f);
}

TEST(Parser, RejectsCorruptCRC) {
    Control c{0.5f, 0.0f};
    uint8_t out[64];
    size_t m = encode(PKT_CONTROL, &c, sizeof c, out);
    out[m - 1] ^= 0xFF;

    Parser p;
    uint8_t type, len;
    uint8_t payload[260];
    for (size_t i = 0; i < m; i++) {
        EXPECT_FALSE(p.feed(out[i], type, payload, len));
    }
}

TEST(Parser, ResyncAfterGarbage) {
    Control c{1.0f, 0.0f};
    uint8_t out[64];
    size_t m = encode(PKT_CONTROL, &c, sizeof c, out);

    Parser p;
    uint8_t type, len;
    uint8_t payload[260];
    uint8_t garbage[] = {0x00, 0xFF, 0xA5, 0x00};
    for (auto b : garbage) p.feed(b, type, payload, len);

    bool got = false;
    for (size_t i = 0; i < m; i++) {
        if (p.feed(out[i], type, payload, len)) { got = true; break; }
    }
    EXPECT_TRUE(got);
}

TEST(Parser, TelemetryRoundTrip) {
    Telemetry tel{};
    tel.t_ms = 1234;
    tel.x = 100.0f; tel.y = 200.0f; tel.z = 150.0f;
    tel.speed = 20.0f; tel.dir = 0.5f;

    uint8_t out[64];
    size_t m = encode(PKT_TELEMETRY, &tel, sizeof tel, out);

    Parser p;
    uint8_t type, len;
    uint8_t payload[260];
    bool got = false;
    for (size_t i = 0; i < m; i++) {
        if (p.feed(out[i], type, payload, len)) { got = true; break; }
    }
    ASSERT_TRUE(got);
    EXPECT_EQ(type, PKT_TELEMETRY);
    Telemetry tel2;
    std::memcpy(&tel2, payload, sizeof tel2);
    EXPECT_EQ(tel2.t_ms, tel.t_ms);
    EXPECT_NEAR(tel2.x, tel.x, 1e-4f);
    EXPECT_NEAR(tel2.z, tel.z, 1e-4f);
}
