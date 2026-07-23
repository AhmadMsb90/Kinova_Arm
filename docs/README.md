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




## Repository Directory Structure

```text
docs/
├── README.md                              # This documentation index
├── docker-kinova-setup.md                 # Environment creation & workspace compilation guide
└── simulation_and_moveit_verification.md  # Simulation testing and execution guide
```
