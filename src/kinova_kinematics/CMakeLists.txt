cmake_minimum_required(VERSION 3.8)
project(kinova_kinematics)



if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 1. Find all required dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(moveit_core REQUIRED)

# 2. Define the executable and its source files
add_executable(kinematics_monitor
  src/kinematics_monitor.cpp
)

# 3. Link dependencies to  executable
ament_target_dependencies(kinematics_monitor
  rclcpp
  geometry_msgs
  moveit_ros_planning_interface
  moveit_core
)

# 4. Handle installation rules
install(TARGETS kinematics_monitor
  DESTINATION lib/${PROJECT_NAME}
)

# 5. Testing configuration 
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()


# Install launch files
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)



ament_package()
