# Real Kinova Gen3 — Vision Module and Camera TF



## 1. Enabling the Physical Kinova Vision Model

The existing Kinova Gen3 Xacro supports the built-in vision module, but the vision argument defaults to false.

In: `src/ros2_kortex/kortex_description/robots/gen3.xacro` the relevant argument is:

```bash
<xacro:arg name="vision" default="false" />
```

The `Gen3 7-DoF Xacro` contains the following `camera frames` when `vision:=true`:
- camera_link 
- camera_depth_frame 
- camera_color_frame

The `physical-robot camera mounting` is defined in:

```yaml
src/ros2_kortex/kortex_description/arms/gen3/7dof/urdf/gen3_macro.xacro
```

For the `non-Gazebo configuration`:

```xml
<xacro:unless value="${sim_gazebo}">
  <link name="${prefix}camera_link" />
  <joint name="${prefix}camera_module" type="fixed">
    <origin xyz="0 0.05639 -0.00305" rpy="${pi} ${pi} 0" />
    <parent link="${prefix}end_effector_link" />
    <child link="${prefix}camera_link" />
  </joint>
```
Therefore, the `camera` is modeled as a `fixed frame` attached to the robot's `end_effector_link`.

## 2. Modifiying the launch files

### 2.1. `gen3.launch.py`
The standard `gen3.launch.py` does `not pass the vision argument` to `kortex_control.launch.py`. The launch arguments are therefore extended with: 
```yaml
"vision": "true", 
```

The relevant section is to be modified as below to enable the vision model when launching the physical Gen3.:

```yaml
launch_arguments={ "robot_type": robot_type, 
                    "robot_ip": robot_ip, 
                    "dof": dof, 
                    "vision": "true", <----------
                    "use_fake_hardware": use_fake_hardware, 
                    ... 
                    "description_file": "gen3.xacro", 
                    }.items(),
```

### 2.2. `kortex_control.launch.py`
The `kortex_control.launch.py` originally did `not declare` or forward a `vision` argument, therefore, `three changes` were made:

1. Declaring the vision launch argument

```bash
declared_arguments.append( 
    DeclareLaunchArgument( 
        "vision", 
        default_value="false", 
        description="Enable the Kinova vision module.", 
    ) 
)
```
2. Reading the launch argument

- Inside the `launch_setup()`:
```bash
vision = LaunchConfiguration("vision")
```

3. Pass vision to Xacro

The `robot_description_content` Xacro command was extended with:

```bash
"vision:=",
vision,
" ",
```

so that the generated robot description receives `vision:=true` and the resulting chain brecomes as below:

```bash
gen3.launch.py
  │
  │ vision=true
  ▼
kortex_control.launch.py
  │
  │ vision:=true
  ▼
gen3.xacro
  │
  ▼
Gen3 robot_description
  └── camera_link
      ├── camera_color_frame
      └── camera_depth_frame

```

## 3. Rebuilding `kortex_bringup` package After

After modifying the Kortex bringup launch files, the `kortex_bringup` package should be rebuilt:

``` bash
cd /workspaces/kinova_ws

colcon build --packages-select kortex_bringup

source install/setup.bash

```

## 4. Verifying Xacro Camera Generation

To be able to rely on the running robot description, the Xacro should be tested directly.

The following command generates the URDF without moving or communicating with the physical robot:

```bash
ros2 run xacro xacro \
  src/ros2_kortex/kortex_description/robots/gen3.xacro \
  vision:=true \
  dof:=7 \
  arm:=gen3 | grep -o "camera_link" | head
  ```

An the result below confirmes that `gen3.xacro + vision:=true`
generates the `camera links` correctly:

```bash
camera_link
camera_link
```


## 5. Launching the Physical arm With Vision Enabled


The physical Kinova Gen3 arm is launched while it was held stationary:

```bash
ros2 launch kortex_bringup gen3.launch.py \
  robot_ip:=192.168.1.10 \
  vision:=true
```


## 6. Verifying the `Camera Model` in `robot_state_publisher`

The camera model is checked in the active `robot_state_publisher` parameter:

```bash
ros2 param get /robot_state_publisher robot_description | \
grep -o "camera_link" | head
```

And the result below confirmes that the running robot description contains the camera model:

```bash
camera_link
camera_link
```

## 7. Verifying the `End-Effector to Camera TF`

The connection between the robot end effector and the camera is checked with:

```bash
ros2 run tf2_ros tf2_echo end_effector_link camera_link
```


The final successful transform is:

``` bash
Translation: [0.000, 0.056, -0.003]
Rotation: in Quaternion (xyzw) [-0.000, -0.000, 1.000, -0.000]
Rotation: in RPY (radian) [-0.000, 0.000, 3.142]
Rotation: in RPY (degree) [-0.000, 0.000, 180.000]

Corresponding transformation matrix:

-1.000  0.000 -0.000  0.000
 0.000 -1.000 -0.000  0.056
-0.000 -0.000  1.000 -0.003
 0.000  0.000  0.000  1.000
```

Therefore, the camera is confirmed to be rigidly attached to the robot's end effector with the modeled physical-camera mounting transform.


## 8. ARM + Camera TF Structure

The relevant part of the physical robot TF tree will be:

```bash
base_link
   │
   └── ... Kinova Gen3 joints ...
          │
          └── bracelet_link
                 │
                 └── end_effector_link
                        │
                        └── camera_link
                               ├── camera_color_frame
                               └── camera_depth_frame
```

The `camera pose` can therefore be related to the `robot base` using the robot's `joint-state TF chain` together with the `fixed camera-to-end-effector transform`.


## 9. Camera and TF Verification

First launching the Kinova vision driver:

```bash
ros2 launch kinova_vision kinova_vision.launch.py
```

The available camera topics included:

```bash
ros2 topic list | grep camera
```

```yaml
/camera/color/camera_info
/camera/color/image_raw
/camera/color/image_raw/compressed
/camera/color/image_raw/compressedDepth
/camera/color/image_raw/theora
/camera/depth/camera_info
/camera/depth/image_raw
/camera/depth/image_raw/compressed
/camera/depth/image_raw/compressedDepth
/camera/depth/image_raw/theora
/camera/image_raw/zstd
```

 the camera image topics were verified with:


The color and depth images can be visually confirmed to be available using `rqt_image_view` by selecting and displaying the following topics:
- `/camera/color/image_raw`
- `/camera/depth/image_raw`

```bash
ros2 run rqt_image_view rqt_image_view
```

## 10. Checking the Active Nodes

The running nodes are checked:


```bash
ros2 node list | grep -E "kinova_vision|robot_state"

```

The following results show that both Kinova `color/depth vision nodes` and the `robot state publisher` were running:

```yaml
/camera/kinova_vision_color
/camera/kinova_vision_depth
/robot_state_publisher
```

### 11. Camera TF Verification

The `transform` from the robot `end effector to the color camera frame` is checked:

```bash
ros2 run tf2_ros tf2_echo end_effector_link camera_color_frame
```

The resulting transform:

```text
Translation: [0.000, 0.056, -0.003]
Rotation: in RPY (degree) [-0.000, 0.000, 180.000]
```

The `transform` from the robot `end effector to the camera depth frame` is checked:

```bash
ros2 run tf2_ros tf2_echo end_effector_link camera_depth_frame
```

The resulting transform was:

```text
Translation: [0.028, 0.066, -0.003]
Rotation: in RPY (degree) [-0.000, 0.000, 180.000]
```

Therefore, the active camera TF structure is:

```text
end_effector_link
├── camera_color_frame
└── camera_depth_frame
```



```bash


```

```bash


```

```bash


```

```bash


```

```bash


```

```bash


```
