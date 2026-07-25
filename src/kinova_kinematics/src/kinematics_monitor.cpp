#include <rclcpp/rclcpp.hpp> // ROS 2 core client library
#include <moveit/move_group_interface/move_group_interface.hpp> // MoveIt 2 robot interface
#include <geometry_msgs/msg/pose.hpp> // ROS 2 3D geometry pose message
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <chrono>


using namespace std::chrono_literals;

class KinematicsMonitor : public rclcpp::Node
{
public:
    KinematicsMonitor()
    : Node("kinematics_monitor", rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true))
    {
        // Initialization happens in run_monitor
    }

    void run_monitor()
    {
        using moveit::planning_interface::MoveGroupInterface;

        // Initialize MoveGroup for the "manipulator" group 
        move_group_ = std::make_shared<MoveGroupInterface>(shared_from_this(), "manipulator");

        // Create a wall timer that triggers printing callback every 1 second (1Hz)
        timer_ = this->create_wall_timer(1s, std::bind(&KinematicsMonitor::timer_callback, this));
        
        RCLCPP_INFO(this->get_logger(), "Kinematics monitor initialized. Printing at 1Hz...");
    }

private:
    void timer_callback()
    {
        // 1. Fetch and print joint angles
        std::vector<double> joint_values = move_group_->getCurrentJointValues();
        RCLCPP_INFO(this->get_logger(), "----------------------------------------");
        RCLCPP_INFO(this->get_logger(), "Current joints (rad):");
        for(size_t i = 0; i < joint_values.size(); ++i)
        {
            RCLCPP_INFO(this->get_logger(), "  Joint %zu: %f", i + 1, joint_values[i]);
        }

        // 2. Fetch and print Cartesian End-Effector spatial pose
        geometry_msgs::msg::Pose pose = move_group_->getCurrentPose().pose;
        RCLCPP_INFO(this->get_logger(), "End Effector Position (meters):");
        RCLCPP_INFO(this->get_logger(), "  x = %f", pose.position.x);
        RCLCPP_INFO(this->get_logger(), "  y = %f", pose.position.y);
        RCLCPP_INFO(this->get_logger(), "  z = %f", pose.position.z);



        RCLCPP_INFO(this->get_logger(), "End Effector Orientation (Quaternion):");
        RCLCPP_INFO(this->get_logger(), "  qx = %.4f", pose.orientation.x);
        RCLCPP_INFO(this->get_logger(), "  qy = %.4f", pose.orientation.y);
        RCLCPP_INFO(this->get_logger(), "  qz = %.4f", pose.orientation.z);
        RCLCPP_INFO(this->get_logger(), "  qw = %.4f", pose.orientation.w);



        tf2::Quaternion q(
        pose.orientation.x,
        pose.orientation.y,
        pose.orientation.z,
        pose.orientation.w);

        double roll, pitch, yaw;

        tf2::Matrix3x3(q).getRPY(roll, pitch, yaw);

        RCLCPP_INFO(this->get_logger(), "End Effector Orientation (RPY):");
        RCLCPP_INFO(this->get_logger(), "  Roll  = %.4f rad", roll);
        RCLCPP_INFO(this->get_logger(), "  Pitch = %.4f rad", pitch);
        RCLCPP_INFO(this->get_logger(), "  Yaw   = %.4f rad", yaw);

    }

    std::shared_ptr<moveit::planning_interface::MoveGroupInterface> move_group_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<KinematicsMonitor>();
    node->run_monitor();

    // spin to keep the node alive and the timer callbacks running indefinitely
    rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}
