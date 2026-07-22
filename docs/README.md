# Kinova ROS 2 Jazzy Integration & Simulation

Welcome to the official repository for running and managing Kinova robot arms using ROS 2 Jazzy. This repository contains the configuration, workspace setups, and deployment tools required for simulation and hardware control.

---

## Quick Navigation

* **[Installation & Setup](#documentation)**: Get your environment and workspace up and running.
* **[Usage](#usage)**: Quick commands to launch simulations and nodes *(Coming Soon)*.
* **[Architecture](#architecture)**: High-level overview of the packages used *(Coming Soon)*.

---

## Documentation

Comprehensive guides for setting up, configuring, and running the project are organized within the `docs/` directory. As the project evolves, additional guides will be added here.

### Environment & Workspace Setup
* **[Docker Image & Kinova Workspace Setup](docs/docker-kinova-setup.md)**  
  Detailed instructions on building the Ubuntu 24.04 / ROS 2 Jazzy Docker container, resolving dependency paths, configuring MoveIt, and compiling the Kinova workspace from source.

*(Future documentation modules—such as simulation launches, hardware bringup, and controller tuning—will be indexed here as they are added.)*

---

## Project Structure

```text
├── docs/
│   └── docker-kinova-setup.md    # Docker & Kinova workspace build guide
├── README.md                     # Main repository documentation
└── ...                           # Workspace source packages (added via setup)