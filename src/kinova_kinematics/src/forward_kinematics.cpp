#include <iostream>
#include <rclcpp/rclcpp.hpp> // Include the ROS 2 C++ client library
#include <moveit/robot_model_loader/robot_model_loader.hpp> // Include the MoveIt! robot model loader
#include <moveit/robot_model/robot_model.hpp> // Include the MoveIt robot model
#include <moveit/robot_state/robot_state.hpp> // Include the MoveIt robot state
#include <Eigen/Geometry> // Include the Eigen library for linear algebra
#include <sensor_msgs/msg/joint_state.hpp> // Include the ROS 2 message type for joint states

class ForwardKinematics : public rclcpp::Node  // Create a ROS 2 node
{
public:
    ForwardKinematics() : Node("forward_kinematics") // Initialize the node with the name "forward_kinematics"
    {
        RCLCPP_INFO(this->get_logger(), "Forward Kinematics initialized.");
    }

    void initialize_model()
    {
        // Now shared_from_this() is completely safe to use
        auto robot_model_loader = std::make_shared<robot_model_loader::RobotModelLoader>(
            shared_from_this(), 
            "robot_description"); 

        robot_model_ = robot_model_loader->getModel(); // Get the robot model from the robot model loader
        if (!robot_model_) {
            RCLCPP_ERROR(this->get_logger(), "Failed to load robot model. Is 'robot_description' published?");
            return;
        }

        RCLCPP_INFO(this->get_logger(), 
                    "Robot model loaded successfully: %s", 
                    robot_model_->getName().c_str());


        auto joint_model_group = robot_model_->getJointModelGroup("manipulator"); // Get the joint model group for the manipulator from the robot model
        if (!joint_model_group) {
            RCLCPP_ERROR(this->get_logger(), "Failed to get joint model group 'manipulator'");
            return; 
            }
        RCLCPP_INFO(this->get_logger(), "Joint model group 'manipulator' loaded successfully.");

        joint_state_sub_ = this->create_subscription<sensor_msgs::msg::JointState>(
            "/joint_states", 10, 
            [this](const sensor_msgs::msg::JointState::SharedPtr msg) {
                this->joint_state_callback(msg);
            }
            ); // Subscribe to the joint state topic with a queue size of 10 and bind the callback function

        
        
        // compute_fk(); // compute forward kinematics
    }


    void compute_fk(const sensor_msgs::msg::JointState::SharedPtr msg)
    {
        // function to compute forward kinematics 
        robot_state_ = std::make_shared<moveit::core::RobotState>(robot_model_); // Create a RobotState object using the robot model
        robot_state_->setToDefaultValues(); // Set the robot state to default values
        robot_state_->setVariablePositions(msg->name, msg->position); // Set the joint positions in the robot state using the received joint state message
        robot_state_->update(); // Update the robot state to reflect the new joint positions
        RCLCPP_INFO(this->get_logger(), "Robot state updated with new joint positions.");

        const Eigen::Isometry3d &end_effector_transform = robot_state_->getGlobalLinkTransform("bracelet_link"); // Get the global transform of the end effector link
        RCLCPP_INFO_STREAM(this->get_logger(), "End effector transform:\n" <<
                   end_effector_transform.matrix().format(Eigen::IOFormat(Eigen::StreamPrecision, 0, ", ", "\n", "[", "]")));

        
        double x = end_effector_transform.translation().x(); // Get the x-coordinate of the end effector
        double y = end_effector_transform.translation().y(); // Get the y-coordinate of the end effector
        double z = end_effector_transform.translation().z(); // Get the z-coordinate of the end effector
        RCLCPP_INFO(this->get_logger(), "End effector position: x = %f, y = %f, z = %f", x, y, z); // Print the end effector position

    }

    void joint_state_callback(const sensor_msgs::msg::JointState::SharedPtr msg) // callback to handle incoming joint state messages
    {  
        RCLCPP_INFO(this->get_logger(), 
                    "Received joint state message with %zu joints.", 
                    msg->name.size());// Print the number of joints in the message


        compute_fk(msg); // Compute forward kinematics whenever a new joint state message is received
    }

private:
    moveit::core::RobotModelPtr robot_model_; // Pointer to the robot model  
    moveit::core::RobotStatePtr robot_state_; // Robot state object to hold the current state of the robot 
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub_; // Subscription to the joint state topic

};



int main(int argc, char** argv) 
{
    rclcpp::init(argc, argv); // Initialize ROS 2 with command line arguments
    auto node = std::make_shared<ForwardKinematics>(); // Create an instance of the ForwardKinematics node
    RCLCPP_INFO(node->get_logger(), "Forward Kinematics Node has started.");
    node->initialize_model(); // Call the method to initialize the robot model
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;

}