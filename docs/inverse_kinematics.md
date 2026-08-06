# Inverse Kinematics Node

The `inverse_kinematics` node computes the required joint configuration of the Kinova Gen3 robotic arm to reach a desired end-effector pose.

The complete pipeline is:

```
Target End-Effector Pose
          |
          v
   Inverse Kinematics Solver
          |
          v
 Joint Position Configuration
   (joint_1 ... joint_7)
          |
          v
 JointTrajectory Message
          |
          v
 Joint Trajectory Controller
          |
          v
 Robot Motion
```

The node does not perform trajectory planning. It only computes the final joint angles required to reach the desired pose. The interpolation and motion execution are handled by the ROS 2 joint trajectory controller.


# Software Components Used

The node uses:

- ROS 2 (`rclcpp`) for node creation and communication.
- MoveIt 2 robot model utilities for loading the Kinova robot description.
- MoveIt 2 `RobotState` for storing the robot configuration.
- MoveIt 2 kinematics plugins for solving inverse kinematics.
- Eigen library for representing transformations.
- `geometry_msgs/msg/PoseStamped` for receiving target end-effector poses.
- `trajectory_msgs/msg/JointTrajectory` for sending joint commands to the controller.


# Launch Configuration

The inverse kinematics node is started using a custom ROS 2 launch file.

The launch file loads the MoveIt configuration:

- `robot_description`
- `robot_description_semantic`
- `robot_description_kinematics`

and passes them as parameters to the node.

Launch command:

```bash
ros2 launch kinova_kinematics ik.launch.py
```

The node is normally started after:

1. Kinova robot bringup
2. MoveIt initialization
3. Inverse kinematics node


Execution order:

```
Kinova Bringup
        |
        v
MoveIt Configuration
        |
        v
Inverse Kinematics Node
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
Create Pose Subscriber
    |
    v
Create Trajectory Publisher
    |
    v
Wait for Target Pose
```


# Loading the Robot Model

The robot model is loaded from:

```
robot_description
```

The model contains:

- Robot links
- Robot joints
- Kinematic chain
- Joint limits
- Link transformations


The loaded robot model is:

```
Kinova Gen3
```


# Selecting the Manipulator Group

The inverse kinematics solver uses the MoveIt planning group:

```
manipulator
```

This group represents the 7-DOF robotic arm:

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

The robot state represents the current configuration of the robot and is required by MoveIt for kinematic calculations.




# Kinematics Solver

The Kinova Gen3 MoveIt configuration uses:

```
KDLKinematicsPlugin
```

for inverse kinematics computation.

The solver receives the desired End-Effector pose and searches for  
Joint Angles ($\theta_1$, $ \ \theta_2$,  ... $ \ \theta_7$)


# Receiving Target End-Effector Pose

The node subscribes to `/target_pose` with message type `PoseStamped` which contains:

## Position

```
x
y
z
```

## Orientation

Quaternion:

```
x
y
z
w
```

Example target pose:

```
Position:

x = 0.5
y = 0.0
z = 0.5


Orientation:

x = 0.0
y = 0.0
z = 0.0
w = 1.0
```

which represents an identity orientation.


# Pose Conversion

MoveIt's IK function requires:

```
Eigen::Isometry3d
```

while ROS communication uses:

```
geometry_msgs::msg::PoseStamped
```

Therefore, the received message is converted:

```
PoseStamped
     |
     v
Position + Quaternion
     |
     v
Eigen::Isometry3d
     |
     v
MoveIt IK Solver
```


# Inverse Kinematics Computation

The IK calculation is performed using:

```cpp
robot_state_->setFromIK(
    joint_model_group_,
    target_pose_,
    "bracelet_link",
    0.1
);
```




# Joint Trajectory Generation

After solving IK, the resulting joint positions are inserted into a message: `trajectory_msgs/msg/JointTrajectory` which contains:

## Joint Names

```
joint_1
joint_2
joint_3
joint_4
joint_5
joint_6
joint_7
```


## Target Joint Positions

The IK solution is stored in:

```
points[0].positions
```


## Execution Time

The trajectory point contains:

```
time_from_start
```


This means the controller should reach the target joint configuration in `time_from_start` seconds.


# Publishing the Trajectory

The trajectory is published to:

```
/joint_trajectory_controller/joint_trajectory
```

with Message type:

`
trajectory_msgs/msg/JointTrajectory
`

This topic is an input interface of the joint trajectory controller.

The controller subscribes to this topic and executes received trajectories.


# Target Pose Publisher Example

A target pose can be published using:

```bash
ros2 topic pub -r 10 /target_pose geometry_msgs/msg/PoseStamped "{
  header: {
    frame_id: 'base_link'
  },
  pose: {
    position: {
      x: 0.5,
      y: 0.0,
      z: 0.5
    },
    orientation: {
      x: 0.0,
      y: 0.0,
      z: 0.0,
      w: 1.0
    }
  }
}"
```


# Checking Generated Joint Trajectory

To verify the generated trajectory:

```bash
ros2 topic echo /joint_trajectory_controller/joint_trajectory --once
```

Expected output:

```
joint_names:
- joint_1
- joint_2
- joint_3
- joint_4
- joint_5
- joint_6
- joint_7

points:
- positions:
  - theta1
  - theta2
  - theta3
  - theta4
  - theta5
  - theta6
  - theta7
```


# Checking Controller Execution

The controller state can be monitored using:

```bash
ros2 topic echo /joint_trajectory_controller/controller_state --once
```

The controller state shows:

- Reference joint positions
- Current feedback positions
- Position error
- Execution status


# Final System Architecture

```
User
 |
 | publishes target pose
 v
/target_pose
 |
 v
Inverse Kinematics Node
 |
 | Solve IK
 v
Joint Angles
 |
 | Create JointTrajectory
 v
/joint_trajectory_controller/joint_trajectory
 |
 v
Joint Trajectory Controller
 |
 v
Kinova Gen3 Robot
```

