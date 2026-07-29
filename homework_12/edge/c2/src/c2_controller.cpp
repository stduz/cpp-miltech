#include "c2_controller.hpp"
#include "fc_link.hpp"
#include "udp_socket.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>
#include <string>

static constexpr uint16_t STUB_PORT = 14560;

static std::string state_str(C2State s) {
    switch (s) {
    case C2State::DISARMED:     return "DISARMED";
    case C2State::ARMED_HOLD:   return "ARMED_HOLD";
    case C2State::ARMED_GUIDED: return "ARMED_GUIDED";
    case C2State::ARMED_MANUAL: return "ARMED_MANUAL";
    }
    return "UNKNOWN";
}

struct C2Controller::Impl {
    C2State state = C2State::DISARMED;
    FcLink fc;
    UdpSocket sock{STUB_PORT};
    std::ofstream log_file;
    bool hold_sent = false;
    bool healthy = false;

    explicit Impl(uint16_t fc_port) : fc(fc_port) {
        log_file.open("/var/log/c2/c2.log", std::ios::app);
    }

    void log(const std::string& msg) {
        std::cout << msg << "\n" << std::flush;
        if (log_file.is_open())
            log_file << msg << "\n" << std::flush;
    }

    void transition(C2State next) {
        if (next == state) return;
        log("[C2] state: " + state_str(state) + " -> " + state_str(next));
        state = next;
        hold_sent = false;
    }
};

C2Controller::C2Controller(uint16_t fc_port)
    : impl_(std::make_unique<Impl>(fc_port))
{}

C2Controller::~C2Controller() = default;

void C2Controller::tick() {
    if (!impl_->healthy && impl_->fc.is_connected()) {
        std::ofstream("/tmp/c2_healthy").close();
        impl_->healthy = true;
    }

    if (!impl_->fc.is_armed()) {
        impl_->transition(C2State::DISARMED);
    } else {
        switch (impl_->fc.flight_mode()) {
        case FcLink::FlightMode::Guided: impl_->transition(C2State::ARMED_GUIDED); break;
        case FcLink::FlightMode::Hold:   impl_->transition(C2State::ARMED_HOLD);   break;
        default:                          impl_->transition(C2State::ARMED_MANUAL); break;
        }
    }

    if (impl_->state == C2State::ARMED_HOLD && !impl_->hold_sent) {
        impl_->fc.hold();
        impl_->hold_sent = true;
    }

    char buf[512];
    ssize_t n = impl_->sock.recv(buf, sizeof(buf) - 1);
    if (n <= 0) return;
    buf[n] = '\0';

    if (impl_->state == C2State::ARMED_GUIDED) {
        try {
            auto j = nlohmann::json::parse(std::string(buf, static_cast<size_t>(n)));
            float north = j.at("north_m").get<float>();
            float east  = j.at("east_m").get<float>();
            impl_->log("[C2] fwd: north=" + std::to_string(north) + " east=" + std::to_string(east));
            impl_->fc.go_to_ned(north, east);
        } catch (const std::exception& e) {
            impl_->log("[C2] error: bad waypoint: " + std::string(e.what()));
        }
    } else {
        impl_->log("[C2] blocked: waypoint in " + state_str(impl_->state));
    }
}

C2State C2Controller::current_state() const {
    return impl_->state;
}
