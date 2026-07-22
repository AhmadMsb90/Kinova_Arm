# Docker Image and Kinova ROS 2 Jazzy Workspace Setup Documentation

This document describes the complete and final procedure used to prepare the Docker environment and build the Kinova ROS 2 Jazzy workspace successfully.

The steps below represent the final working procedure after resolving dependency, header path, and package configuration issues. Trial-and-error steps are omitted.

---

# Part 1 — Docker Image Setup (Ubuntu 24.04 + ROS 2 Jazzy)

The Kinova simulation environment was prepared inside a Docker image based on Ubuntu 24.04 with ROS 2 Jazzy Desktop.

## 1. Pull Ubuntu 24.04 Base Image

```bash
docker pull ubuntu:24.04
```

---

## 2. Create Initial Container

```bash
docker run -it ubuntu:24.04 bash
```

---

## 3. Update the System

Inside the container:

```bash
apt update
apt upgrade -y
```

Install required development tools:

```bash
apt install -y \
    curl \
    wget \
    git \
    gnupg2 \
    lsb-release \
    software-properties-common \
    build-essential \
    cmake \
    python3-pip \
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-vcstool
```

---

## 4. Add ROS 2 Repository

Install repository tools:

```bash
apt update

apt install -y software-properties-common curl

add-apt-repository universe
```

Add ROS key:

```bash
curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
    -o /usr/share/keyrings/ros-archive-keyring.gpg
```

Add ROS repository:

```bash
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] \
http://packages.ros.org/ros2/ubuntu \
$(. /etc/os-release && echo $UBUNTU_CODENAME) main" \
> /etc/apt/sources.list.d/ros2.list
```

---

## 5. Install ROS 2 Jazzy Desktop

```bash
apt update

apt install -y ros-jazzy-desktop
```

---

## 6. Install ROS Development Dependencies

```bash
apt install -y \
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-vcstool \
    python3-argcomplete
```

---

## 7. Initialize rosdep

```bash
rosdep init

rosdep update
```

---

## 8. Source ROS Automatically

Add ROS sourcing to `.bashrc`:

```bash
echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc

source ~/.bashrc
```

---

## 9. Create Reusable Docker Image

After configuring the container:

```bash
docker commit <container_name> ubuntu_24_jazzy:latest
```

This creates a reusable image containing Ubuntu 24.04 and ROS 2 Jazzy.

---

# Running the ROS 2 Jazzy Container

The container was started with GPU support, GUI forwarding for RViz/Gazebo, and a mounted host workspace.

Generic command:

```bash
docker run -it --rm \
    --privileged \
    --cap-add SYS_ADMIN \
    --device /dev/fuse \
    --gpus all \
    -e DISPLAY=$DISPLAY \
    -e QT_X11_NO_MITSHM=1 \
    -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
    -v <host_workspace_directory>:<container_workspace_directory> \
    --name <container_name> \
    <docker_image_name> \
    bash
```


---

## Docker Run Options Explanation

- `--privileged`
  - Provides extended permissions required by robotics applications and hardware-related packages.

- `--cap-add SYS_ADMIN`
  - Adds administrative capabilities required by some system-level operations.

- `--device /dev/fuse`
  - Enables FUSE support.

- `--gpus all`
  - Provides access to NVIDIA GPUs inside the container.

- `-e DISPLAY=$DISPLAY`
  - Enables GUI applications through the host X server.

- `-e QT_X11_NO_MITSHM=1`
  - Prevents Qt shared-memory problems with Docker GUI applications.

- `-v /tmp/.X11-unix:/tmp/.X11-unix:rw`
  - Shares the X11 socket for applications such as RViz and Gazebo.

- `-v <host>:<container>`
  - Mounts a persistent workspace from the host system.

- `--rm`
  - Removes the container after exit.

- `--name`
  - Assigns a custom container name.

---

# Part 2 — Kinova ROS 2 Jazzy Workspace Setup

The Kinova ROS 2 Kortex workspace was built from source inside the Docker container.

Workspace:

```bash
mkdir -p ~/kinova_ws/src

cd ~/kinova_ws
```

---

# 1. Clone Required Source Repositories


```bash
cd ~/kinova_ws/src

git clone https://github.com/Kinovarobotics/ros2_kortex.git

git clone -b jazzy https://github.com/moveit/moveit2.git

git clone -b jazzy https://github.com/ros-controls/ros2_control.git

git clone -b jazzy https://github.com/ros-controls/ros2_controllers.git
```

Additional dependencies were imported using the repository `.repos` file when required.

---

# 2. Install ROS Dependencies

Update package lists:

```bash
apt update
```

Update rosdep:

```bash
rosdep update
```

Install dependencies:

```bash
rosdep install \
    --from-paths src \
    --ignore-src \
    -r -y
```

---

# 3. Install Additional Required Packages

Some dependencies require manual installation:

```bash
apt install -y \
    ros-jazzy-trajectory-msgs \
    ros-jazzy-test-msgs \
    ros-jazzy-diagnostic-msgs \
    ros-jazzy-ament-cmake-google-benchmark \
    ros-jazzy-ros-testing \
    ros-jazzy-parameter-traits
```

---

# 4. Fix Parameter Traits Header Path

During compilation, `ackermann_steering_controller` may fail because the installed header location may not match the include statement.

Installed location:

```
/opt/ros/jazzy/include/parameter_traits/parameter_traits/parameter_traits.hpp
```

Expected include:

```cpp
#include <parameter_traits/parameter_traits.hpp>
```

Create compatibility symbolic link:

```bash
ln -s \
/opt/ros/jazzy/include/parameter_traits/parameter_traits/parameter_traits.hpp \
/opt/ros/jazzy/include/parameter_traits/parameter_traits.hpp
```

Verify:

```bash
ls -l /opt/ros/jazzy/include/parameter_traits
```

Expected structure:

```
parameter_traits/
parameter_traits.hpp -> parameter_traits/parameter_traits/parameter_traits.hpp
```

---

# 5. Fix MoveIt Test Package Environment Generation

The package:

```
src/moveit2/moveit_ros/tests
```

may not generate the required file:

```
local_setup.bash
```



Cause:

The possible package missing:

```cmake
ament_package()
```

in:

```
src/moveit2/moveit_ros/tests/CMakeLists.txt
```

Add:

```cmake
ament_package()
```

after the existing CMake commands.

After this modification, the package is correctly generated:

```
install/moveit_ros_tests/share/moveit_ros_tests/local_setup.bash
```

---

# 6. Clean Workspace Before Final Build

```bash
cd ~/kinova_ws

rm -rf build
rm -rf install
rm -rf log
```

---

# 7. Source ROS 2 Jazzy

```bash
source /opt/ros/jazzy/setup.bash
```

---

# 8. Build Complete Workspace

Final build command:

```bash
colcon build \
    --symlink-install \
    --parallel-workers 3
```

Successful result:

```
Summary: 138 packages finished
0 packages failed
0 packages aborted
```

---

# 9. Source Workspace

```bash
source install/setup.bash
```

No missing package setup files should appear.

---

# 10. Verify Installation

Check Kinova packages:

```bash
ros2 pkg list | grep kortex
```

Expected packages:

```
kortex_api
kortex_bringup
kortex_description
kortex_driver
```

Check MoveIt:

```bash
ros2 pkg list | grep moveit
```

Check ros2_control:

```bash
ros2 pkg list | grep ros2_control
```

---

# Notes

## Harmless Warnings

The following warnings can appear during compilation but do not indicate failure:

```
SetuptoolsDeprecationWarning:
setup.py install is deprecated.
```

and:

```
You did not request a specific build type:
Choosing 'Release' for maximum performance
```

These are normal build messages.

Having this done, the Docker environment and Kinova ROS 2 Jazzy workspace are successfully configured:

The workspace will be ready for:

- Kinova Gen3 simulation
- Gazebo integration
- MoveIt motion planning
- ros2_control execution
- RViz visualization

Start a new session with:

```bash
source /opt/ros/jazzy/setup.bash

source ~/kinova_ws/install/setup.bash
```

and proceed with the Kinova simulation launch tests.