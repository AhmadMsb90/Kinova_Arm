# Kinova Gen3 Forward Kinematics Monitoring with MoveIt 2 (ROS 2 Jazzy)

## Overview

This package implements a C++ ROS 2 node for monitoring the kinematic state of the Kinova Gen3 7-DOF robotic arm.

The node uses:

- ROS 2 Jazzy
- MoveIt 2
- MoveGroupInterface (C++)
- Kinova Gen3 MoveIt configuration
- Fake hardware simulation

The purpose of this package is to programmatically access:

- Current joint positions (joint space)
- Current end-effector Cartesian pose (task space)

The node demonstrates the connection between:

- Joint states from the robot simulation
- MoveIt kinematic model
- Forward kinematics computation


 

# Package Structure

The package is located inside:

```text
kinova_ws/src/kinova_kinematics
```

Structure:

```text
kinova_kinematics
|
|-- CMakeLists.txt
|
|-- package.xml
|
|-- launch
|   `-- kinematics.launch.py
|
|-- include
|   `-- kinova_kinematics
|
`-- src
    `-- kinematics_monitor.cpp
```


 

# Implementation Description

## kinematics.launch.py

The launch file loads the Kinova Gen3 MoveIt configuration and passes the required robot descriptions to the monitoring node.

The following parameters are loaded:

- `robot_description`
    - URDF robot model

- `robot_description_semantic`
    - SRDF semantic robot description
    - Includes planning groups and end-effector definitions

- `robot_description_kinematics`
    - Kinematics solver configuration


The launch file starts:

```text
kinova_kinematics/kinematics_monitor
```


 

## kinematics_monitor.cpp

The node uses:

```cpp
moveit::planning_interface::MoveGroupInterface
```

to communicate with MoveIt.


The MoveIt planning group is initialized as:

```cpp
MoveGroupInterface(shared_from_this(), "manipulator")
```

This gives access to:

- Joint values
- End-effector pose
- Robot state


The node runs a timer callback at:

```text
1 Hz
```

and prints:

### Joint Space

The current seven joint angles:

```text
joint_1
joint_2
joint_3
joint_4
joint_5
joint_6
joint_7
```


### Cartesian Space

The current end-effector position:

```text
x
y
z
```

obtained from MoveIt's forward kinematics.


 

# Building the Package

From the workspace root:

```bash
cd /workspaces/kinova_ws
```

Source ROS 2:

```bash
source /opt/ros/jazzy/setup.bash
```

Build the package:

```bash
colcon build \
--symlink-install \
--packages-select kinova_kinematics
```


Source the workspace:

```bash
source install/setup.bash
```


 

# Running the Complete Simulation Pipeline

The complete system requires three terminals.

 

# Terminal 1: Start Kinova Fake Hardware

Source ROS 2 and workspace:

```bash
source /opt/ros/jazzy/setup.bash
source /workspaces/kinova_ws/install/setup.bash
```


Launch the Kinova Gen3 fake hardware:

```bash
ros2 launch kortex_bringup gen3.launch.py \
robot_ip:=127.0.0.1 \
use_fake_hardware:=true
```


This starts:

- Kinova robot description
- ros2_control hardware interface
- Joint state publisher
- Fake robot controller


The robot model becomes available through ROS 2 topics and TF.


 

# Terminal 2: Start MoveIt Motion Planning

Source ROS 2 and workspace:

```bash
source /opt/ros/jazzy/setup.bash
source /workspaces/kinova_ws/install/setup.bash
```


Launch the MoveIt configuration:

```bash
ros2 launch kinova_gen3_7dof_robotiq_2f_85_moveit_config robot.launch.py \
robot_ip:=127.0.0.1
```


This starts:

- MoveIt planning node
- RViz visualization
- Motion Planning panel
- Robot state monitoring


The arm can now be moved:

1. Open RViz.

2. Select:

```text
MotionPlanning
```

3. Select planning group:

```text
manipulator
```

4. Move the interactive marker.

5. Press:

```text
Plan
```

6. Press:

```text
Execute
```


The simulated robot moves and the joint states change.


 

# Terminal 3: Start Kinematics Monitor Node

Source ROS 2 and workspace:

```bash
source /opt/ros/jazzy/setup.bash
source /workspaces/kinova_ws/install/setup.bash
```


Launch the kinematics monitor:

```bash
ros2 launch kinova_kinematics kinematics.launch.py
```


The node continuously prints:


## Current Joint Values

Example:

```text
Current joints (rad):

Joint 1: 0.000000
Joint 2: 0.000000
Joint 3: 0.000000
Joint 4: 0.000000
Joint 5: 0.000000
Joint 6: 0.000000
Joint 7: 0.000000
```


## End Effector Position

Example:

```text
End Effector Position (meters):

x = 0.000000
y = 0.000000
z = 0.000000
```


When the arm is moved in RViz, these values update automatically.


 
