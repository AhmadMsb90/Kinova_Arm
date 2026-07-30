# Inverse Kinematics Node

The `inverse_kinematics` node is responsible for computing the required joint configuration of the Kinova Gen3 robotic arm to reach a desired end-effector pose.

The node uses:

- ROS 2 (`rclcpp`) for node creation and execution.
- MoveIt 2 robot model utilities for loading the robot description.
- MoveIt 2 `RobotState` for representing the current robot configuration.
- MoveIt 2 kinematics plugins for solving the inverse kinematics problem.
- The Kinova Gen3 manipulator planning group defined in the SRDF.

The objective of inverse kinematics is the opposite of forward kinematics.

```
Desired End-Effector Pose
        |
        v
Inverse Kinematics Solver
        |
        v
Joint Configuration
(joint_1 ... joint_7)
```

# Node Initialization



The initialization sequence is:

```
ROS 2 Node
    |
    v
Load Robot Model
    |
    v
Select Manipulator Group
    |
    v
Create Robot State
    |
    v
Compute Inverse Kinematics
```

# Loading the Robot Model

The robot model is loaded from the MoveIt robot description:

```
robot_description
```

The loaded model contains:

- Robot links
- Robot joints
- Kinematic structure
- Joint limits
- Transform relationships


# Selecting the Manipulator Group

The inverse kinematics solver uses the manipulator planning group `manipulator`.

This group corresponds to the 7-DOF robotic arm:

```
joint_1
joint_2
joint_3
joint_4
joint_5
joint_6
joint_7
```

# Robot State Creation

A MoveIt `RobotState` object is created from the loaded robot model.

The RobotState represents the current configuration of the robot and is required by MoveIt for kinematic calculations.

The state is initialized using the default robot configuration.

# Kinematics Solver Configuration


The Kinova Gen3 configuration uses `KDLKinematicsPlugin`for solving the inverse kinematics problem.

After loading the kinematics configuration, MoveIt creates the solver for the manipulator group.

# Inverse Kinematics Computation

The inverse kinematics calculation receives a desired end-effector pose.

The target pose contains:

- Cartesian position:
  - x coordinate
  - y coordinate
  - z coordinate

- End-effector orientation:
  - rotation matrix

The IK solver searches for a joint configuration:

```
Target End-Effector Pose
          |
          v
   KDL IK Solver
          |
          v
Joint Angles Solution
(joint_1 ... joint_7)
```

The target pose used for testing is:

```
x = 0.5
y = 0.0
z = 0.5
```

with an identity orientation.
