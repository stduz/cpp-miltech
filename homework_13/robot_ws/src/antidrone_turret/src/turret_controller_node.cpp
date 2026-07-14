#include "antidrone_turret/turret_logic.hpp"
#include "antidrone_turret/msg/target.hpp"
#include "antidrone_turret/msg/actuator_status.hpp"
#include "antidrone_turret/msg/turret_status.hpp"
#include "antidrone_turret/msg/gimbal_command.hpp"
#include "antidrone_turret/msg/servo_command.hpp"
#include "antidrone_turret/srv/trigger_actuator.hpp"
#include <rclcpp/rclcpp.hpp>

using namespace antidrone_turret;
using ActuatorStatus = antidrone_turret::msg::ActuatorStatus;
using Target = antidrone_turret::msg::Target;
using TurretStatusMsg = antidrone_turret::msg::TurretStatus;
using GimbalCommandMsg = antidrone_turret::msg::GimbalCommand;
using ServoCommandMsg = antidrone_turret::msg::ServoCommand;
using TriggerActuator = antidrone_turret::srv::TriggerActuator;

class TurretControllerNode : public rclcpp::Node {
public:
    TurretControllerNode() : Node("turret_controller_node") {
        float thr = declare_parameter("confidence_threshold", 0.8f);
        float max_d = declare_parameter("max_distance_m", 30.0f);
        logic_ = std::make_unique<TurretLogic>(thr, max_d);

        target_sub_ = create_subscription<Target>(
            "/perception/target", 10,
            std::bind(&TurretControllerNode::on_target, this, std::placeholders::_1));

        actuator_sub_ = create_subscription<ActuatorStatus>(
            "/actuator/status", 10,
            std::bind(&TurretControllerNode::on_actuator_status, this, std::placeholders::_1));

        gimbal_pub_  = create_publisher<GimbalCommandMsg>("/gimbal/cmd", 10);
        servo_pub_   = create_publisher<ServoCommandMsg>("/servo/cmd", 10);
        status_pub_  = create_publisher<TurretStatusMsg>("/turret/status", 10);

        trigger_client_ = create_client<TriggerActuator>("/actuator/trigger");
    }

private:
    void on_actuator_status(const ActuatorStatus::SharedPtr msg) {
        actuator_state_ = (msg->state == ActuatorStatus::READY)
            ? ActuatorState::kReady : ActuatorState::kReloading;
    }

    void on_target(const Target::SharedPtr msg) {
        auto dec = logic_->decide(msg->visible, msg->x, msg->y,
                                   msg->distance_m, msg->confidence, actuator_state_);

        TurretStatusMsg status;
        status.target_state  = static_cast<uint8_t>(dec.target_state);
        status.action        = static_cast<uint8_t>(dec.action);
        status.trigger_state = static_cast<uint8_t>(dec.trigger);
        status.confidence    = dec.confidence;
        status.distance_m    = dec.distance_m;
        status_pub_->publish(status);

        if (dec.action != TurretAction::Track) return;

        auto sc = logic_->servo_command(msg->x);
        ServoCommandMsg servo_msg;
        servo_msg.direction = sc.direction;
        servo_msg.target_x  = sc.target_x;
        servo_msg.error_x   = sc.error_x;
        servo_pub_->publish(servo_msg);

        auto gc = logic_->gimbal_command(msg->y);
        GimbalCommandMsg gimbal_msg;
        gimbal_msg.direction = gc.direction;
        gimbal_msg.target_y  = gc.target_y;
        gimbal_msg.error_y   = gc.error_y;
        gimbal_pub_->publish(gimbal_msg);

        if (dec.trigger == TriggerDecision::Requested) {
            auto req = std::make_shared<TriggerActuator::Request>();
            req->confidence = msg->confidence;
            req->distance_m = msg->distance_m;
            trigger_client_->async_send_request(req,
                [this](rclcpp::Client<TriggerActuator>::SharedFuture f) {
                    auto r = f.get();
                    RCLCPP_INFO(get_logger(), "[turret] trigger: accepted=%d count=%u",
                                r->accepted, r->trigger_count);
                });
        }
    }

    std::unique_ptr<TurretLogic> logic_;
    ActuatorState actuator_state_{ActuatorState::kReady};

    rclcpp::Subscription<Target>::SharedPtr target_sub_;
    rclcpp::Subscription<ActuatorStatus>::SharedPtr actuator_sub_;
    rclcpp::Publisher<GimbalCommandMsg>::SharedPtr gimbal_pub_;
    rclcpp::Publisher<ServoCommandMsg>::SharedPtr servo_pub_;
    rclcpp::Publisher<TurretStatusMsg>::SharedPtr status_pub_;
    rclcpp::Client<TriggerActuator>::SharedPtr trigger_client_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TurretControllerNode>());
    rclcpp::shutdown();
    return 0;
}
