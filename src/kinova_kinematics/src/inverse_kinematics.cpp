#include <iostream>
#include <rclcpp/rclcpp.hpp> // Include the ROS 2 C++ client library
#include <moveit/robot_model_loader/robot_model_loader.hpp> // Include the MoveIt
#include <moveit/robot_model/robot_model.hpp> // Include the MoveIt robot model
#include <moveit/robot_state/robot_state.hpp> // Include the MoveIt robot state
#include <Eigen/Geometry> // Include the Eigen library for linear algebra
#include <sensor_msgs/msg/joint_state.hpp> // Include the ROS 2 message type for
#include <geometry_msgs/msg/pose_stamped.hpp> // Include the ROS 2 message type for pose
#include <tf2_ros/transform_broadcaster.hpp> // Include the TF2 transform broadcaster
#include <trajectory_msgs/msg/joint_trajectory.hpp> // Include the ROS 2 message type for joint trajectory
#include <chrono>


using namespace std::chrono_literals;

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

        target_pose_ = Eigen::Isometry3d::Identity(); // Initialize the target pose as an identity transformation


        joint_trajectory_pub_ = this->create_publisher<trajectory_msgs::msg::JointTrajectory>(
            "/joint_trajectory_controller/joint_trajectory",
             10); // Create a publisher for joint trajectories

        pose_sub_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
            "/target_pose",
            10,
            std::bind(&InverseKinematics::pose_callback, this, std::placeholders::_1)); // Create a subscription for target poses
    

    }

    // ----- Inverse Kinematics Computation -----
    void compute_ik() // Method to compute inverse kinematics
    {
    

        bool success = robot_state_->setFromIK(joint_model_group_, target_pose_, "bracelet_link", 0.1); // Compute the inverse kinematics

        if (success) 
        {
            RCLCPP_INFO(this->get_logger(), "Inverse kinematics computation succeeded!");
        } 
        else 
        {
            RCLCPP_ERROR(this->get_logger(), "Inverse kinematics computation failed!");
            return;
        }

        std::vector<double> joint_positions; // Vector to hold the joint positions
        robot_state_->copyJointGroupPositions(joint_model_group_, joint_positions); // Copy the joint positions from the robot state to the vector
        
        for (std::size_t i = 0; i < joint_positions.size(); ++i) 
        {
            RCLCPP_INFO(this->get_logger(), 
                        "Joint %zu | Position: %f", 
                        i + 1, joint_positions[i]); 
        }

        joint_trajectory_msg_.header.stamp = this->now(); // Set the timestamp of the joint trajectory message
        joint_trajectory_msg_.header.frame_id = "base_link"; // Set the frame ID of the joint trajectory message
        joint_trajectory_msg_.joint_names = joint_model_group_->getVariableNames(); // Set the joint names of the joint trajectory message
        joint_trajectory_msg_.points.clear();
        joint_trajectory_msg_.points.resize(1); // Resize the points vector to hold one point
        joint_trajectory_msg_.points[0].positions = joint_positions; // Set the positions of the first point to the computed joint positions
        joint_trajectory_msg_.points[0].time_from_start = rclcpp::Duration(3, 0); // Set the time from start for the first point to 3 second


    }
    void pose_callback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) // Callback for pose messages
    {
        //Receive the target pose 
        if (!msg) {
            RCLCPP_ERROR(this->get_logger(), "Received null pose message!");
            return;
        }

        RCLCPP_INFO(this->get_logger(), 
                    "Received target pose: Position(%f, %f, %f), Orientation(%f, %f, %f, %f)", 
                    msg->pose.position.x, 
                    msg->pose.position.y, 
                    msg->pose.position.z, 
                    msg->pose.orientation.x, 
                    msg->pose.orientation.y, 
                    msg->pose.orientation.z, 
                    msg->pose.orientation.w); // Log the received pose

        target_pose_.translation().x() = msg->pose.position.x; // Set the x-coordinate of the target pose
        target_pose_.translation().y() = msg->pose.position.y; // Set the y-coordinate of the target pose
        target_pose_.translation().z() = msg->pose.position.z; // Set the z-coordinate of the target pose
        Eigen::Quaterniond q(msg->pose.orientation.w, 
                             msg->pose.orientation.x, 
                             msg->pose.orientation.y, 
                             msg->pose.orientation.z); // Create a quaternion from the received orientation
        target_pose_.linear() = q.toRotationMatrix(); // Set the orientation of the target pose

        compute_ik(); // Compute the inverse kinematics for the received pose

        joint_trajectory_pub_->publish(joint_trajectory_msg_); // Publish the joint trajectory message

    }


private:
    moveit::core::RobotModelPtr robot_model_; // The robot model
    moveit::core::RobotStatePtr robot_state_; // The robot state
    moveit::core::JointModelGroup* joint_model_group_; // The joint model group
    rclcpp::Publisher<trajectory_msgs::msg::JointTrajectory>::SharedPtr joint_trajectory_pub_; 
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr pose_sub_; // Subscription to the EE target pose 
    trajectory_msgs::msg::JointTrajectory joint_trajectory_msg_; // The joint trajectory message
    Eigen::Isometry3d target_pose_; // The target pose

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