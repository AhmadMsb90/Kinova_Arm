// ----- Header Files -----
#include <iostream>
#include <rclcpp/rclcpp.hpp> // Include the ROS 2 C++ client library
#include <moveit/robot_model_loader/robot_model_loader.hpp> // Include the MoveIt! robot model loader
#include <moveit/robot_model/robot_model.hpp> // Include the MoveIt robot model
#include <moveit/robot_state/robot_state.hpp> // Include the MoveIt robot state
#include <Eigen/Geometry> // Include the Eigen library for linear algebra
#include <sensor_msgs/msg/joint_state.hpp> // Include the ROS 2 message type for joint states
#include <geometry_msgs/msg/pose_stamped.hpp> // Include the ROS 2 message type for pose
#include <tf2_ros/transform_broadcaster.hpp> // Include the TF2 transform broadcaster


// ----- Forward Kinematics Node -----

class ForwardKinematics : public rclcpp::Node  // Create a ROS 2 node
{
public:
    ForwardKinematics() : Node("forward_kinematics") // Initialize the node 
    {
        RCLCPP_INFO(this->get_logger(), "Forward Kinematics initialized.");
    }

    // ----- Robot Model Initialization -----

    void initialize_model()
    {

        auto robot_model_loader = std::make_shared<robot_model_loader::RobotModelLoader>(
            shared_from_this(), 
            "robot_description"); // Create a robot model loader

        robot_model_ = robot_model_loader->getModel(); // Get the robot model 

        if (!robot_model_) {
            RCLCPP_ERROR(this->get_logger(), "Failed to load robot model. Is 'robot_description' published?");
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

        joint_state_sub_ = this->create_subscription<sensor_msgs::msg::JointState>(
            "/joint_states", 10, 
            [this](const sensor_msgs::msg::JointState::SharedPtr msg) {
                this->joint_state_callback(msg);
            }
            ); // Subscribe to the joint state topic 

        pose_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>(
            "/fk_pose", 10); // Create a publisher for the end effector pose 

        tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this); // Create a transform broadcaster to publish transform  


        
    }

    // ----- Forward Kinematics Computation -----

    void compute_fk() // Method to compute forward kinematics 
    {
        
        
        robot_state_->update(); // Update the robot state to reflect the new joint positions

        const Eigen::Isometry3d &end_effector_transform = 
                        robot_state_->getGlobalLinkTransform("bracelet_link"); // Get the global transform of the end effector link

        double x = end_effector_transform.translation().x(); // Get the x-coordinate of the end effector
        double y = end_effector_transform.translation().y(); // Get the y-coordinate of the end effector
        double z = end_effector_transform.translation().z(); // Get the z-coordinate of the end effector
        RCLCPP_INFO(this->get_logger(), "End effector position: x = %f, y = %f, z = %f", x, y, z); 

        geometry_msgs::msg::PoseStamped pose_msg; // Create a PoseStamped message to publish the end effector pose
        pose_msg.header.frame_id = "base_link"; // Set the frame ID to "base_link"
        pose_msg.header.stamp = this->now(); // Set the timestamp to the current time
        pose_msg.pose.position.x = x; // Set the x-coordinate of the pose
        pose_msg.pose.position.y = y; // Set the y-coordinate of the pose
        pose_msg.pose.position.z = z; // Set the z-coordinate of the pose

        Eigen::Quaterniond quaternion(end_effector_transform.rotation()); // Convert the rotation matrix to a quaternion
        pose_msg.pose.orientation.x = quaternion.x();
        pose_msg.pose.orientation.y = quaternion.y();
        pose_msg.pose.orientation.z = quaternion.z();
        pose_msg.pose.orientation.w = quaternion.w();

        pose_pub_->publish(pose_msg); // Publish the end effector pose 

        geometry_msgs::msg::TransformStamped transform_msg; // Create a TransformStamped message to publish the end effector transform
        transform_msg.header.frame_id = "base_link"; // Set the frame ID to "base_link"
        transform_msg.header.stamp = this->now(); // Set the timestamp to the current time
        transform_msg.child_frame_id = "fk_end_effector"; // Set the child frame ID to "fk_end_effector"

        transform_msg.transform.translation.x = x; // Set the x-coordinate of the transform
        transform_msg.transform.translation.y = y; // Set the y-coordinate of the transform
        transform_msg.transform.translation.z = z; // Set the z-coordinate of the transform     

        transform_msg.transform.rotation.x = quaternion.x(); // Set the x-coordinate of the rotation
        transform_msg.transform.rotation.y = quaternion.y(); // Set the y-coordinate of the rotation
        transform_msg.transform.rotation.z = quaternion.z(); // Set the z-coordinate of the rotation    
        transform_msg.transform.rotation.w = quaternion.w(); // Set the w-coordinate of the rotation

        tf_broadcaster_->sendTransform(transform_msg); // Publish the end effector transform

        RCLCPP_INFO(this->get_logger(),
            "Joint 2 = %f",
            robot_state_->getVariablePosition("joint_2"));

        RCLCPP_INFO(this->get_logger(),
            "Joint 4 = %f",
            robot_state_->getVariablePosition("joint_4"));

    }


    // ----- Joint State Callback -----

    void joint_state_callback(const sensor_msgs::msg::JointState::SharedPtr msg) // callback to handle incoming joint state messages
    {  
        
        robot_state_->setVariablePositions(msg->name, msg->position); // Set the joint positions in the robot state


        compute_fk(); // Compute forward kinematics when a new joint state message is received
    }

private:

    // ----- Class Member Variables -----

    moveit::core::RobotModelPtr robot_model_; // Pointer to the robot model  
    moveit::core::RobotStatePtr robot_state_; // Robot state object to hold the current state of the robot 
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub_; // Subscription to the joint state topic
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pose_pub_; // Publisher for the end effector pose
    std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_; // Transform broadcaster to publish the end effector transform
    const moveit::core::JointModelGroup* joint_model_group_;
};



// ----- Main -----

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