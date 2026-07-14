#include "antidrone_turret/msg/gimbal_command.hpp"
#include <rclcpp/rclcpp.hpp>

using GimbalCommand = antidrone_turret::msg::GimbalCommand;

static const char* dir_str(int8_t d) {
    if (d > 0) return "UP";
    if (d < 0) return "DOWN";
    return "CENTER";
}

class GimbalDriverNode : public rclcpp::Node {
public:
    GimbalDriverNode() : Node("gimbal_driver_node") {
        sub_ = create_subscription<GimbalCommand>(
            "/gimbal/cmd", 10,
            [this](const GimbalCommand::SharedPtr msg) {
                RCLCPP_INFO(get_logger(),
                    "gimbal_driver_node отримав: direction=%s target_y=%.0f error_y=%.0f",
                    dir_str(msg->direction), static_cast<double>(msg->target_y),
                    static_cast<double>(msg->error_y));
            });
    }
private:
    rclcpp::Subscription<GimbalCommand>::SharedPtr sub_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<GimbalDriverNode>());
    rclcpp::shutdown();
    return 0;
}
