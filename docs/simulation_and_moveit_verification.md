## Kinova Gen3 ROS 2 Jazzy Simulation Bringup and MoveIt Verification

### Start Kinova Gen3 Fake Hardware Simulation

After successfully building the ROS 2 Jazzy Kinova workspace, source the workspace:

```bash
source /workspaces/kinova_ws/install/setup.bash
```

Launch the Kinova Gen3 robot using fake hardware:

```bash
ros2 launch kortex_bringup gen3.launch.py \
  robot_ip:=127.0.0.1 \
  use_fake_hardware:=true
```

The robot model appears in RViz with the MoveIt visualization interface.

The `robot_ip` argument is required by the launch file even when using fake hardware.


## Verify ros2_control Controllers

Open another terminal inside the same container.

Source ROS 2 and the workspace:

```bash
source /opt/ros/jazzy/setup.bash
source /workspaces/kinova_ws/install/setup.bash
```

Check the available controllers:

```bash
ros2 control list_controllers
```

Expected output:

```text
joint_state_broadcaster     joint_state_broadcaster/JointStateBroadcaster          active
twist_controller            picknik_twist_controller/PicknikTwistController        inactive
joint_trajectory_controller joint_trajectory_controller/JointTrajectoryController  active
```

The active controllers confirm that:

- Joint state publishing is running.
- Joint trajectory commands can be sent.
- The fake hardware interface is connected.

---

## Verify Joint State Feedback

Check the published joint states:

```bash
ros2 topic echo /joint_states
```

Expected output:

```text
header:
  stamp:
    sec: ...
    nanosec: ...
  frame_id: base_link

name:
- joint_1
- joint_2
- joint_3
- joint_4
- joint_5
- joint_6
- joint_7

position:
- 0.0
- 0.0
- 0.0
- 0.0
- 0.0
- 0.0
- 0.0

velocity:
- 0.0
- 0.0
- 0.0
- 0.0
- 0.0
- 0.0
- 0.0

effort:
- 0.0
- 0.0
- 0.0
- 0.0
- 0.0
- 0.0
- 0.0
```


## MoveIt Motion Planning Test in RViz

1. Open the RViz window started by the Kinova launch file.

2. In the left panel:

```
MotionPlanning
```

should be visible.

3. Open:

```
MotionPlanning → Planning
```

4. Select the planning group:

```
arm
```


5. The robot end-effector interactive marker appears.

6. Move the interactive marker:
   - Drag the colored arrows/rings.
   - Place the end-effector in a reachable target pose.

7. Click:

```
Plan
```

MoveIt generates a collision-free trajectory.

8. If planning succeeds, click:

```
Execute
```

The simulated Kinova arm moves in RViz and the joint positions update.

---

## Returning the Robot to Home Position

In RViz:

1. Open:

```
MotionPlanning → Planning
```

2. Find:

```
Planning Request
```

3. Open:

```
Select Start State
```

4. Choose:

```
Current
```

to use the current robot state.

5. Open:

```
Select Goal State
```

6. Choose:

```
Home
```


7. Click:

```
Plan
```

8. Click:

```
Execute
```

The robot returns to the predefined home configuration.

