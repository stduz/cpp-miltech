#include "antidrone_turret/msg/servo_command.hpp"
#include <rclcpp/rclcpp.hpp>

using ServoCommand = antidrone_turret::msg::ServoCommand;

static const char* dir_str(int8_t d) {
    if (d > 0) return "RIGHT";
    if (d < 0) return "LEFT";
    return "CENTER";
}

class YawServoDriverNode : public rclcpp::Node {
public:
    YawServoDriverNode() : Node("yaw_servo_driver_node") {
        sub_ = create_subscription<ServoCommand>(
            "/servo/cmd", 10,
            [this](const ServoCommand::SharedPtr msg) {
                RCLCPP_INFO(get_logger(),
                    "yaw_servo_driver_node отримав: direction=%s target_x=%.0f error_x=%.0f",
                    dir_str(msg->direction), static_cast<double>(msg->target_x),
                    static_cast<double>(msg->error_x));
            });
    }
private:
    rclcpp::Subscription<ServoCommand>::SharedPtr sub_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<YawServoDriverNode>());
    rclcpp::shutdown();
    return 0;
}
