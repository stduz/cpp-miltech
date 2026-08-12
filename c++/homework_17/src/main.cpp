#include <common/mavlink.h>
#include "nav/Navigator.h"
#include "drone_link.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <csignal>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <thread>

using namespace dlink;
using clk = std::chrono::steady_clock;

static constexpr float  DT          = 0.1f;
static constexpr float  MAX_SPEED   = 20.0f;
static constexpr float  MAX_ACCEL   = 5.0f;
static constexpr float  MAX_ANGULAR = 0.5f;
static constexpr float  ALT         = 100.0f;
static constexpr double LAT0        = 50.4501;
static constexpr double LON0        = 30.5234;
static constexpr double M_PER_DEG   = 111320.0;

static constexpr uint8_t  SYSID   = 1;
static constexpr uint8_t  COMPID  = MAV_COMP_ID_AUTOPILOT1;
static constexpr uint16_t CMD_DROP = MAV_CMD_USER_1;
static constexpr int      MAX_RETRIES = 5;
static constexpr int      ACK_TIMEOUT_MS = 1000;

static volatile bool g_running = true;
static void sig_handler(int) { g_running = false; }

struct State {
    float x = 0, y = 0, z = ALT;
    float vx = 0, vy = 0, speed = 0;
    float dir = 0.785f;
    uint32_t t_ms = 0;
};

static void to_gps(double x, double y, double& lat, double& lon) {
    double lat0_rad = LAT0 * M_PI / 180.0;
    lat = LAT0 + y / M_PER_DEG;
    lon = LON0 + x / (M_PER_DEG * std::cos(lat0_rad));
}

static int make_udp(const char* dest_ip, uint16_t dest_port, sockaddr_in& dest) {
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) { perror("socket"); return -1; }

    dest = {};
    dest.sin_family = AF_INET;
    dest.sin_port   = htons(dest_port);
    inet_pton(AF_INET, dest_ip, &dest.sin_addr);

    sockaddr_in local{};
    local.sin_family      = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port        = htons(14551);
    bind(s, (sockaddr*)&local, sizeof local);

    int fl = fcntl(s, F_GETFL, 0);
    fcntl(s, F_SETFL, fl | O_NONBLOCK);
    return s;
}

static void mav_send(int s, const sockaddr_in& dest, mavlink_message_t& msg) {
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    int n = (int)mavlink_msg_to_send_buffer(buf, &msg);
    sendto(s, buf, n, 0, (sockaddr*)&dest, sizeof dest);
}

static void send_heartbeat(int s, const sockaddr_in& dest) {
    mavlink_message_t msg;
    mavlink_msg_heartbeat_pack(SYSID, COMPID, &msg,
        MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC,
        MAV_MODE_FLAG_SAFETY_ARMED, 0, MAV_STATE_ACTIVE);
    mav_send(s, dest, msg);
}

static void send_position(int s, const sockaddr_in& dest, const State& st) {
    double lat, lon;
    to_gps(st.x, st.y, lat, lon);

    // NED: vx=north=vy_local, vy=east=vx_local
    float ned_vx_cms = st.vy * 100.0f;
    float ned_vy_cms = st.vx * 100.0f;

    // hdg: compass, 0=north, 9000=east (cdeg)
    double hdg_deg = std::fmod(90.0 - st.dir * 180.0 / M_PI + 360.0, 360.0);
    uint16_t hdg = (uint16_t)(hdg_deg * 100.0);

    mavlink_message_t msg;
    mavlink_msg_global_position_int_pack(SYSID, COMPID, &msg,
        st.t_ms,
        (int32_t)(lat * 1e7),
        (int32_t)(lon * 1e7),
        (int32_t)(st.z * 1000.0f),
        (int32_t)(st.z * 1000.0f),
        (int16_t)ned_vx_cms,
        (int16_t)ned_vy_cms,
        0,
        hdg);
    mav_send(s, dest, msg);
}

static void send_attitude(int s, const sockaddr_in& dest, const State& st) {
    // NED yaw: 0=north(+y), π/2=east(+x) → yaw_ned = π/2 - dir_math
    float yaw = (float)(M_PI / 2.0) - st.dir;
    while (yaw >  (float)M_PI) yaw -= 2.0f * (float)M_PI;
    while (yaw < -(float)M_PI) yaw += 2.0f * (float)M_PI;

    mavlink_message_t msg;
    mavlink_msg_attitude_pack(SYSID, COMPID, &msg,
        st.t_ms, 0.0f, 0.0f, yaw, 0.0f, 0.0f, 0.0f);
    mav_send(s, dest, msg);
}

static void send_command_long(int s, const sockaddr_in& dest, const State& st, uint8_t confirm) {
    double lat, lon;
    to_gps(st.x, st.y, lat, lon);

    mavlink_message_t msg;
    mavlink_msg_command_long_pack(SYSID, COMPID, &msg,
        0, 0,
        CMD_DROP,
        confirm,
        0, 0, 0, 0,
        (float)lat,
        (float)lon,
        st.z);
    mav_send(s, dest, msg);
}

static bool try_recv_ack(int s, uint16_t expected_cmd) {
    uint8_t buf[512];
    mavlink_message_t msg;
    mavlink_status_t  status{};
    ssize_t n = recv(s, buf, sizeof buf, 0);
    if (n <= 0) return false;
    for (int i = 0; i < (int)n; i++) {
        if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &msg, &status)) {
            if (msg.msgid == MAVLINK_MSG_ID_COMMAND_ACK) {
                mavlink_command_ack_t ack;
                mavlink_msg_command_ack_decode(&msg, &ack);
                if (ack.command == expected_cmd && ack.result == MAV_RESULT_ACCEPTED)
                    return true;
            }
        }
    }
    return false;
}

int main(int argc, char** argv) {
    const char* dest_ip   = "127.0.0.1";
    uint16_t    dest_port = 14550;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--dest") && i + 1 < argc) {
            const char* p = argv[++i];
            const char* colon = strrchr(p, ':');
            if (colon) {
                static char ip_buf[64];
                size_t len = (size_t)(colon - p);
                memcpy(ip_buf, p, len);
                ip_buf[len] = '\0';
                dest_ip   = ip_buf;
                dest_port = (uint16_t)atoi(colon + 1);
            } else {
                dest_ip = p;
            }
        }
    }

    signal(SIGINT,  sig_handler);
    signal(SIGTERM, sig_handler);

    sockaddr_in dest{};
    int s = make_udp(dest_ip, dest_port, dest);
    if (s < 0) return 1;

    printf("[hw17] sending MAVLink to %s:%u\n", dest_ip, dest_port);

    // Mission setup — VOG-17, 5 targets from config
    AmmoCfg ammo{};
    strncpy(ammo.name, "VOG-17", sizeof ammo.name);
    ammo.mass      = 0.35f;
    ammo.drag      = 0.07f;
    ammo.lift      = 0.0f;
    ammo.hitRadius = 5.0f;
    ammo.nTargets  = 5;

    // Initial target positions (from targets.json t=0)
    TargetPos tgt_init[5] = {
        {0,  90.12f, 201.54f},
        {1, 105.00f, 185.00f},
        {2, 105.97f, 172.05f},
        {3, 101.00f, 198.00f},
        {4,  89.00f, 213.00f},
    };

    Navigator nav;
    nav.setAmmo(ammo);
    for (auto& t : tgt_init) nav.updateTarget(t);

    State st;
    bool  dropped    = false;
    int   drop_tries = 0;
    bool  ack_ok     = false;
    State drop_state{};
    clk::time_point drop_time{};
    clk::time_point last_hb  = clk::now();
    clk::time_point step_due = clk::now();

    while (g_running) {
        auto now = clk::now();

        // Wait for next physics step
        if (now < step_due) {
            std::this_thread::sleep_until(step_due);
            now = clk::now();
        }
        step_due = now + std::chrono::milliseconds((int)(DT * 1000));

        // Build telemetry from state
        Telemetry tel{};
        tel.t_ms  = st.t_ms;
        tel.x     = st.x;
        tel.y     = st.y;
        tel.z     = st.z;
        tel.vx    = st.vx;
        tel.vy    = st.vy;
        tel.speed = st.speed;
        tel.dir   = st.dir;

        // Navigation
        NavCmd cmd = nav.compute(tel);

        // Physics update
        st.speed += cmd.accel * MAX_ACCEL * DT;
        st.speed  = std::clamp(st.speed, 0.0f, MAX_SPEED);
        st.dir   += cmd.turnRate * MAX_ANGULAR * DT;
        st.vx     = st.speed * std::cos(st.dir);
        st.vy     = st.speed * std::sin(st.dir);
        st.x     += st.vx * DT;
        st.y     += st.vy * DT;
        st.t_ms  += (uint32_t)(DT * 1000);

        // Send telemetry (every step = 10Hz ≥ 2Hz requirement)
        send_position(s, dest, st);
        send_attitude(s, dest, st);

        // Heartbeat at 1Hz
        if (std::chrono::duration<double>(now - last_hb).count() >= 1.0) {
            send_heartbeat(s, dest);
            last_hb = now;
        }

        // Drop logic
        if (cmd.drop && !dropped) {
            dropped    = true;
            drop_state = st;
            printf("[hw17] DROP at t=%.2f pos=(%.1f,%.1f) dir=%.2f v=%.1f\n",
                   st.t_ms / 1000.0f, st.x, st.y, st.dir, st.speed);
            send_command_long(s, dest, drop_state, 0);
            drop_tries = 1;
            drop_time  = clk::now();
        }

        if (dropped && !ack_ok && drop_tries <= MAX_RETRIES) {
            if (try_recv_ack(s, CMD_DROP)) {
                printf("[hw17] ACK received (attempt %d)\n", drop_tries);
                ack_ok = true;
            } else {
                double elapsed = std::chrono::duration<double, std::milli>(
                    clk::now() - drop_time).count();
                if (elapsed >= ACK_TIMEOUT_MS) {
                    if (drop_tries < MAX_RETRIES) {
                        drop_tries++;
                        printf("[hw17] retry %d/%d\n", drop_tries, MAX_RETRIES);
                        send_command_long(s, dest, drop_state, (uint8_t)(drop_tries - 1));
                        drop_time = clk::now();
                    } else {
                        printf("[hw17] ACK not received after %d attempts\n", MAX_RETRIES);
                        ack_ok = true;
                    }
                }
            }
        }
    }

    close(s);
    return 0;
}
