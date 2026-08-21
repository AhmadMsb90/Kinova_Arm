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
Any available host IP address within the `192.168.1.X` range (excluding the robot's default IP of `192.168.1.10`) can be chosen to establish communication with the arm. In this case, `192.168.1.11` was selected.




Connection of the physical robot to the computer over LAN can ve verified:

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

## 12. Real-Robot Joint-Trajectory Motion Test

After validating FK on the physical Kinova Gen3, the real robot can be commanded through the ROS 2 `joint_trajectory_controller`.

- The `active controllers` can be checked with:

```bash
ros2 control list_controllers
```

- which gives:
```bash
joint_trajectory_controller  joint_trajectory_controller/JointTrajectoryController  active
joint_state_broadcaster      joint_state_broadcaster/JointStateBroadcaster          active
fault_controller             picknik_reset_fault_controller/PicknikResetFaultController active
twist_controller             picknik_twist_controller/PicknikTwistController      inactive
```

- These confirme that the `joint_trajectory_controller` is `active` and that the `joint_state_broadcaster` is `publishing` the `physical joint states`.
---
- The `controller's command interface` was also inspected:

```bash
ros2 control list_hardware_interfaces
```

- which gives:
```bash
joint_1/position [available] [claimed]
joint_2/position [available] [claimed]
joint_3/position [available] [claimed]
joint_4/position [available] [claimed]
joint_5/position [available] [claimed]
joint_6/position [available] [claimed]
joint_7/position [available] [claimed]
```

---
- The controller configuration can be inspected with:

```bash
ros2 param dump /joint_trajectory_controller
```
- The important configuration is as below which shows that the controller accepts joint-position trajectories for all seven joints.

```bash
command_interfaces:
- position

joints:
- joint_1
- joint_2
- joint_3
- joint_4
- joint_5
- joint_6
- joint_7

state_interfaces:
- position
- velocity

update_rate: 1000
```

---
- The trajectory command topic can be verified:

```bash
ros2 topic info /joint_trajectory_controller/joint_trajectory

Type: trajectory_msgs/msg/JointTrajectory
Publisher count: 0
Subscription count: 1

Node name: joint_trajectory_controller
```

## 13. Direct Joint-Trajectory Test

- First, the controller state can be inspected using:

```bash
ros2 topic echo /joint_trajectory_controller/controller_state --once
```
- Then the controller reportes the current joint positions as both reference/feedback values. 

- **`reference`**: The commanded or desired position (in radians) that the controller is telling each joint to move to.
- **`feedback`**: The actual physical position reported by the joint encoders (sensors) on the robot at that exact moment.

For example: 

```bash
reference:
  joint_1: -0.041960
  joint_2:  0.746750
  joint_3: -3.125713
  joint_4: -2.031514
  joint_5: -0.419832
  joint_6:  1.394533
  joint_7:  1.854343

feedback:
  joint_1: -0.041961
  joint_2:  0.746871
  joint_3: -3.125713
  joint_4: -2.031519
  joint_5: -0.419833
  joint_6:  1.394528
  joint_7:  1.854343

```
- And the very small errors indicated that the controller is tracking its commanded/reference state correctly.

- Then a complete `seven-joint trajectory` can be sent, using the measured current joint configuration as the baseline and changing only `joint_1`. The following trajectory can be sent in which the `joint_1` was intentionally changed only and the remaining six joints were kept essentially unchanged:

```bash
old joint_1 ≈ -0.041960 --> new joint_1 ≈ -0.024506
```

```bash
ros2 topic pub --once /joint_trajectory_controller/joint_trajectory \
trajectory_msgs/msg/JointTrajectory "
{
  joint_names: ['joint_1', 'joint_2', 'joint_3', 'joint_4', 'joint_5', 'joint_6', 'joint_7'],
  points: [
    {
      positions: [
        -0.02450621,
         0.74687074,
        -3.12571276,
        -2.03151920,
        -0.41983350,
         1.39452755,
         1.85434295
      ],
      time_from_start: {sec: 8, nanosec: 0}
    }
  ]
}"

```

After execution, `/joint_states `shows as below which confirms that the physical robot responds to the trajectory command:

```bash
joint_1 = -0.024511
joint_2 =  0.747089
joint_3 = -3.125712
joint_4 = -2.031522
joint_5 = -0.419833
joint_6 =  1.394526
joint_7 =  1.854343
```
## 14. Inverse Kinematics Test

After validating direct joint-space control, the custom inverse-kinematics node is tested.

- The [inverse kinematic node](inverse_kinematics.md) is launched in one terminal and it loades the `Kinova Gen3 MoveIt model`:

```bash
ros2 launch kinova_kinematics ik.launch.py
```

- The [inverse kinematic node](inverse_kinematics.md) uses the `MoveIt manipulator group` and the `end_effector_link` as the `IK tip`.

- The [inverse kinematic node](inverse_kinematics.md) subscribes to the topic `/target_pose` which is of type `geometry_msgs/msg/PoseStamped` and having the targe pose it computes a trajectory message of type `trajectory_msgs/msg/JointTrajectory`:

--- 

- To first inspect the generated IK solution created by the 
[inverse kinematic node](inverse_kinematics.md) without automatically sending it to the physical robot a temporary trajectory publisher topic `/ik_joint_solution` can be used in order to avoid any unwanted, destructive movement of the arm:

- The resulting ROS graph will be:

```bash
/target_pose
      │
      ▼
inverse_kinematics
      │
      ▼
/ik_joint_solution

```

- Checking the trajectory publisher topic: 

```bash
ros2 topic info /ik_joint_solution

Type: trajectory_msgs/msg/JointTrajectory
Publisher count: 1
Subscription count: 0
```

## 15. IK Target-Pose Test on the arm 

Once the performnace of the [inverse kinematic node](inverse_kinematics.md) on the fake topic is check, the real arm can be moved by publishing to the real arm trajectory topic:
 `/joint_trajectory_controller/joint_trajectory"`

- Then the physical robot's current pose of the `end-effector` with respect to the `base-link` of the arm is inspected:

 ```bash
ros2 run tf2_ros tf2_echo base_link end_effector_link

Translation:
x = 0.531
y = -0.153
z = 0.326

Quaternion (xyzw):
x = 0.546
y = 0.350
z = 0.313
w = 0.694
 ```

