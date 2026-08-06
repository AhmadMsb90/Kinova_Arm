# Kinova Gen3 ROS 2 Jazzy Documentation

This directory contains the setup guides, build fixes, and verification procedures for the Kinova Gen3 (7DOF) manipulator using ROS 2 Jazzy, MoveIt, and ros2_control.


## Documentation Index

Follow these guides in sequential order to set up the environment and verify system functionality:

### 1. [Docker Image & Workspace Setup](docker-kinova-setup.md)
* **Environment:** Ubuntu 24.04 and ROS 2 Jazzy Desktop inside Docker with GPU/GUI forwarding.
* **Source Builds:** Compilation logs for ros2_kortex, moveit2, and ros2_control.
* **Critical Bug Fixes:** Resolution for the parameter_traits header path mismatch and moveit_ros_tests CMake build failures.

### 2. [Simulation & MoveIt Verification](simulation_and_moveit_verification.md)
* **Bringup:** Initializing the Kinova Gen3 fake hardware simulation pipeline.
* **Health Checks:** Monitoring active ros2_control states and verifying data flow on /joint_states.
* **Motion Planning:** Validation of collision-free trajectories and home positioning using RViz and MoveIt.

### 3. [Kinematics Monitoring Node](kinova_kinematics_monitor.md)
* **Node Implementation:** C++ ROS 2 node utilizing MoveIt 2 MoveGroupInterface to monitor joint space and task space.
* **Forward Kinematics:** Real-time programmatic access to current joint angles and end-effector Cartesian positions ($x, y, z$).
* **Pipeline Integration:** Instructions to run alongside the simulation and MoveIt verification steps.


### 4. [Forward Kinematics Node Implementation](forward_kinematics.md)
* **Node Purpose:** Computes the Cartesian pose of the Kinova Gen3 end-effector directly from current robot joint positions using MoveIt robot model utilities and `RobotState`.
* **Architecture:** Subscribes to `/joint_states`, builds/updates the internal MoveIt `RobotState`, and extracts the `bracelet_link` transform matrix ($T = [R \ p; 0 \ 1]$).
* **Core Components:** Details on node structure, `initialize_model()`, `joint_state_callback()`, and step-by-step FK calculations.


## Repository Directory Structure

```bash
docs/
├── README.md                              
├── docker-kinova-setup.md                 # Environment creation & workspace compilation guide
├── simulation_and_moveit_verification.md  # Simulation testing and execution guide
├── kinova_kinematics_monitor.md           # Kinematics monitoring node implementation guide[cite: 4]
└── forward_kinematics.md                  # Forward kinematics node implementation details