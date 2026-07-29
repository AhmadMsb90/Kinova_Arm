# Forward Kinematics Node 

The `forward_kinematics` node is responsible for computing the Cartesian pose of the Kinova Gen3 end-effector from the current robot joint positions.

The node uses:

- ROS 2 (`rclcpp`) for node creation and communication.
- MoveIt 2 robot model utilities for loading the robot description.
- MoveIt 2 `RobotState` for representing the current robot configuration.
- `/joint_states` topic as the source of the current joint positions.
- Forward kinematics through MoveIt's kinematic model to compute the end-effector transform.

The data flow is:

```
/joint_states
      |
      |
      v
Forward Kinematics Node
      |
      |
      v
MoveIt RobotState
      |
      |
      v
Forward Kinematics Calculation
      |
      |
      v
End-Effector Position (x,y,z)
```

 

# Node Structure

The node is implemented as a C++ ROS 2 node:

```
ForwardKinematics
        |
        |
        +-- initialize_model()
        |
        +-- joint_state_callback()
        |
        +-- compute_fk()
```
# Node Responsibilities

| Function | Responsibility |
| :--- | :--- |
| **Constructor** | Creates the ROS 2 node |
| **`initialize_model()`** | Loads the robot model and creates the joint state subscriber |
| **`joint_state_callback()`** | Receives updated joint positions from `/joint_states` |
| **`compute_fk()`** | Updates the robot state and calculates the end-effector pose |

 

# ROS 2 Node Initialization

The node is created with the name:

```
forward_kinematics
```

The constructor only initializes the ROS 2 node and prints a startup message.

At this stage no robot model or kinematics calculation is performed.

 

# Loading the Robot Model

The robot model is loaded from the ROS 2 parameter:

```
robot_description
```

This parameter is provided by the MoveIt launch system.

The loading sequence is:

```
robot_description
        |
        v
RobotModelLoader
        |
        v
RobotModel
```

The loaded model contains:

- Robot links
- Robot joints
- Kinematic chains
- Joint limits
- Transform relationships


 

# Selecting the Manipulator Group

The Kinova Gen3 SRDF defines the planning group:

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


 

# Subscribing to Joint States

The node subscribes to:

```
/joint_states
```

This topic is published by `ros2_control`.

The messages contain:

- Joint names
- Joint positions
- Joint velocities
- Joint efforts




Whenever a new joint state message arrives:

```
/joint_states message
        |
        v
joint_state_callback()
        |
        v
compute_fk()
```

Therefore, FK is recalculated every time the robot configuration changes.

 

# Joint State Callback

The callback function is responsible for handling incoming joint updates.

Its tasks are:

1. Receive the `JointState` message.
2. Print the number of received joints.
3. Pass the joint information to the FK calculation.


The callback connects ROS 2 communication with MoveIt kinematics.

 

# Forward Kinematics Calculation

The FK calculation follows these steps:

## 1. Create Robot State

A `RobotState` object is created using the loaded robot model.

The RobotState represents:

```
Robot Model
      +
Joint Values
      |
      v
Current Robot Configuration
```

 

## 2. Set Joint Positions

The joint positions received from  `‍‍‍‍/joint_states‍‍‍‍‍` are copied into the `‍‍‍‍RobotState`‍‍‍‍. Then the `RobotState` represents the actual robot configuration.

 

## 3. Update Robot State

The update operation recalculates all internal transformations.

Conceptually:

```
Joint angles
      |
      v
Forward kinematic chain
      |
      v
Link transformations
```

 

## 4. Extract End-Effector Transform

The node requests the transformation of `bracelet_link`.


The transformation contains:

- Position
- Orientation


The transformation matrix is:

```
T =
[R  p]
[0  1]
```

where:

- `R` represents orientation.
- `p` represents position.

 

 
# Publishing the End-Effector Pose

After computing the forward kinematics, the node publishes the resulting Cartesian pose.

The pose is published as `/fk_pose` in the `base_link` frame using the message type `geometry_msgs/PoseStamped` and contains:

- Header
- Position (x, y, z)
- Orientation (quaternion)



# ROS Interfaces

## Subscribed Topics

| Topic | Message Type | Purpose |
| :--- | :--- | :--- |
| `/joint_states` | `sensor_msgs/JointState` | Receives the current robot joint positions |

## Published Topics

| Topic | Message Type | Purpose |
| :--- | :--- | :--- |
| `/fk_pose` | `geometry_msgs/PoseStamped` | Publishes the computed end-effector pose |

