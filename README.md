# MyosotisFramework
A framework created with Vulkan 

## Requirements
- **[cmake](https://cmake.org)**
- **[Git](https://git-scm.com)**
- vswhere

## Environment Setup (Windows)
To set up the environment, use the provided batch file `envSetup.bat`.  
This file will:
1. Check if `git` and `cmake` are installed on your system.
2. Install them if they are not already installed.
3. Create any required junctions to ensure proper paths and dependencies are set up correctly.

## Build the Solution
Once the environment setup is complete, you can use the following batch file to create the solution file:  
`cmakeBuild.bat`

## Features
- **Hot Reload Support**  
  The project supports hot reloading, allowing changes to be instantly applied after modification.   
  - **Solution Configuration**  
    The solution configuration is set to `FWDLL`. Build the project using this configuration to ensure proper operation.
  - **Reload Method**  
    Press the F5 key to trigger hot reloading.

- **ImGui**  
  ImGui has been integrated into the project, providing an easy-to-use graphical user interface for debugging and development.
