# Real Kinova Gen3 — FK Validation

## Status

**Forward Kinematics validation on the physical Kinova Gen3 arm**


## 1. Hardware / Environment

- Robot: Kinova Gen3
- ROS 2: Jazzy
- OS: Ubuntu 24.04
- Connection: LAN
- Kinova IP: `192.168.1.10`


Running `ip addr` yields the list of system network interfaces. The specific segment corresponding to the Kinova robotic arm is detailed below:

```bash
2: enp0s31f6: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP group default qlen 1000
    link/ether fc:5c:ee:cd:0b:4d brd ff:ff:ff:ff:ff:ff
    inet 192.168.1.11/24 brd 192.168.1.255 scope global noprefixroute enp0s31f6
    inet6 fe80::97d5:b90e:c583:51cf/64 scope link noprefixroute 
```
Any available host IP address within the `192.168.1.X` range (excluding the robot's default IP of `192.168.1.10`) could be chosen to establish communication with the arm. In this case, `192.168.1.11` was selected.




Connection of the physical robot to the computer over LAN could ve verified:

```bash
ping -c 4 192.168.1.10

PING 192.168.1.10 (192.168.1.10) 56(84) bytes of data. 
192.168.1.10: icmp_seq=1 ttl=64 time=0.742 ms 64 bytes from 192.168.1.10: icmp_seq=2 ttl=64 time=0.331 ms
```

## 2. Kinova Bringup

The Kinova ROS 2 bringup is located at:

```bash
src/ros2_kortex/kortex_bringup/launch/gen3.launch.py
```

Using the launch file, the arm can be brought up:

```bash
ros2 launch kortex_bringup gen3.launch.py  robot_ip:=192.168.1.10
```


## 3. Check Joint-State Topics

```bash
ros2 topic list | grep joint

/dynamic_joint_states
/joint_state_broadcaster/transition_event
/joint_states
/joint_trajectory_controller/controller_state
/joint_trajectory_controller/joint_trajectory
/joint_trajectory_controller/speed_scaling_input
/joint_trajectory_controller/transition_event

```

## 4. Check Actual Joint Positions

```bash
ros2 topic echo /joint_states
```

## 5. Check TF Between Base and End Effector

```bash
ros2 run tf2_ros tf2_echo base_link end_effector_link
```

## 6. Check TF Tree

```bash
ros2 run tf2_tools view_frames
```
The TF tree is found to be:

```bash
world
 └── base_link
      └── shoulder_link
           └── half_arm_1_link
                └── half_arm_2_link
                     └── forearm_link
                          └── spherical_wrist_1_link
                               └── spherical_wrist_2_link
                                    └── bracelet_link
                                         └── end_effector_link
                                              └── tool_frame
```

## 7. Check Robot Description and parameter

```bash
ros2 topic list | grep robot_description

/robot_description
```

```bash
ros2 param list | grep robot_description

robot_description
```

## 8. Inspect the robot description

```bash
ros2 topic echo /robot_description --once

data: <?xml version="1.0" ?> ...

```

## 9. Check Important ROS Nodes

```bash
ros2 node list | grep -E "robot_state|controller|kortex"

/controller_manager
/fault_controller
/joint_trajectory_controller
/kortexmultiinterfacehardware
/robot_state_publisher
/twist_controller
```
* **The controller manager is running** (`/controller_manager`)
* **The fault controller is idle/available** (`/fault_controller`)
* **The joint trajectory controller is running** (`/joint_trajectory_controller`)
* **The physical Kortex / Kinova hardware interface is active and running** (`/kortexmultiinterfacehardware`)
* **robot_state_publisher is running** (`/robot_state_publisher`)
* **The Cartesian velocity controller is running** (`/twist_controller`)



## 10. Validation check

Running the arm bring up launch file in a terminal: 

```bash
ros2 launch kortex_bringup gen3.launch.py   robot_ip:=192.168.1.10
```

Running the forward kinematics node in the second terminal: 
```bash
ros2 launch kinova_kinematics fk.launch.py
```
In the third terminal running echo the topic published by the forward kinematics node:

```bash
ros2 topic echo /fk_pose
```
And in the fourth terminal (to get the real-time translation and orientation between the robot's `base` and its `end-effector`): 
```bash
ros2 run tf2_ros tf2_echo base_link end_effector_link
```

The results from the two above should be identical.


## 11. Current ROS Data Flow

At this point the important real-robot data flow is:

```bash
Kinova Gen3
    │
    ├── /joint_states
    │       │
    │       └── actual q1...q7
    │
    ├── TF
    │       │
    │       └── base_link → end_effector_link
    │
    └── /robot_description
            │
            └── robot URDF description
```

The custom kinematics package uses the joint configuration to calculate the end-effector pose:

```bash
q1...q7
   │
   ▼
 FK
   │
   ▼
end-effector pose
```