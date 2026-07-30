#include <iostream>
#include <rclcpp/rclcpp.hpp> // Include the ROS 2 C++ client library
#include <moveit/robot_model_loader/robot_model_loader.hpp> // Include the MoveIt
#include <moveit/robot_model/robot_model.hpp> // Include the MoveIt robot model
#include <moveit/robot_state/robot_state.hpp> // Include the MoveIt robot state
#include <Eigen/Geometry> // Include the Eigen library for linear algebra
#include <sensor_msgs/msg/joint_state.hpp> // Include the ROS 2 message type for
#include <geometry_msgs/msg/pose_stamped.hpp> // Include the ROS 2 message type for pose
#include <tf2_ros/transform_broadcaster.hpp> // Include the TF2 transform broadcaster


// ----- Inverse Kinematics Node -----
class InverseKinematics : public rclcpp::Node  // Create a ROS 2 node
{
public:

    InverseKinematics() : Node("inverse_kinematics") // Initialize the node 
    {
        RCLCPP_INFO(this->get_logger(), "Inverse Kinematics initialized.");
    }

    // ----- Robot Model Initialization -----

    void initialize_model()
    {
        RCLCPP_INFO(this->get_logger(), "Loading robot model...");
        auto robot_model_loader = std::make_shared<robot_model_loader::RobotModelLoader>(
            shared_from_this(),
            "robot_description"); // Create a robot model loader

        robot_model_ = robot_model_loader->getModel(); // Get the robot model

        if (!robot_model_) {
            RCLCPP_ERROR(this->get_logger(), "Failed to load robot model!");
            return;
        }
        RCLCPP_INFO(this->get_logger(), 
                    "Robot model loaded successfully: %s", 
                    robot_model_->getName().c_str()); 

        joint_model_group_ = robot_model_->getJointModelGroup("manipulator"); // Get the joint model group
        if (!joint_model_group_) {
            RCLCPP_ERROR(this->get_logger(), "Failed to get joint model group 'manipulator'");
            return; 
        }
        RCLCPP_INFO(this->get_logger(), "Joint model group 'manipulator' loaded successfully.");

        robot_state_ = std::make_shared<moveit::core::RobotState>(robot_model_); // Create a robot state object

        if(!robot_state_) {
            RCLCPP_ERROR(this->get_logger(), "Failed to create robot state!");
            return;
        }
        RCLCPP_INFO(this->get_logger(), "Robot state created successfully.");

        robot_state_->setToDefaultValues(); // Set the robot state to default values

        RCLCPP_INFO(this->get_logger(), "Robot state set to default values.");

        compute_ik();
        


    }

    // ----- Inverse Kinematics Computation -----
    void compute_ik() // Method to compute inverse kinematics
    {
        // geometry_msgs::msg::PoseStamped target_pose; // PoseStamped message for the target pose
        Eigen::Isometry3d target_pose = Eigen::Isometry3d::Identity(); // Initialize the target pose as an identity transformation

        target_pose.translation().x() = 0.5; // Set the x-coordinate of the target pose
        target_pose.translation().y() = 0.0; // Set the y-coordinate of the target pose
        target_pose.translation().z() = 0.5; // Set the z-coordinate of the target pose
        target_pose.linear() = Eigen::Matrix3d::Identity(); // Set the orientation of the target pose to identity



        bool success = robot_state_->setFromIK(joint_model_group_, target_pose, "bracelet_link", 0.1); // Compute the inverse kinematics

        if (success) 
        {
            RCLCPP_INFO(this->get_logger(), "Inverse kinematics computation succeeded!");
        } 
        else 
        {
            RCLCPP_ERROR(this->get_logger(), "Inverse kinematics computation failed!");
        }
    }



private:
    moveit::core::RobotModelPtr robot_model_; // The robot model
    moveit::core::RobotStatePtr robot_state_; // The robot state
    moveit::core::JointModelGroup* joint_model_group_; // The joint model group




};



int main(int argc, char** argv) 
{
    rclcpp::init(argc, argv); // Initialize ROS 2
    auto node = std::make_shared<InverseKinematics>(); // Instantiate the node
    node->initialize_model(); // Initialize the robot model
    RCLCPP_INFO(node->get_logger(), "Inverse Kinematics Node has started.");
    rclcpp::spin(node); // Spin the node
    rclcpp::shutdown(); // Shutdown ROS 2
    return 0;
}