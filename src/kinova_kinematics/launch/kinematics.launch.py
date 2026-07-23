import os
from launch import LaunchDescription
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder

def generate_launch_description():
    # Automatically extracts URDF and SRDF parameters from the kortex configuration
    moveit_config = MoveItConfigsBuilder("kinova_gen3_7dof_robotiq_2f_85", package_name="kinova_gen3_7dof_robotiq_2f_85_moveit_config").to_moveit_configs()

    return LaunchDescription([
        Node(
            package='kinova_kinematics',
            executable='kinematics_monitor',
            name='kinematics_monitor',
            output='screen',
            parameters=[
                moveit_config.robot_description,          # Maps URDF
                moveit_config.robot_description_semantic, # Maps SRDF (Fixes your exact error)
                moveit_config.robot_description_kinematics,
                {'use_sim_time': True} 
            ]
        )
    ])
