# MyosotisFramework
A framework created with Vulkan 

## Requirements
- **[cmake](https://cmake.org)**
- **[Git](https://git-scm.com)**

## Environment Setup (Windows)
To set up the environment, use the provided batch file `envSetup.bat`.  
This file will:
1. Check if `git` and `cmake` are installed on your system.
2. Install them if they are not already installed.

## Build the Solution
Once the environment setup is complete, you can use the following batch file to create the solution file:  
`cmakeBuild.bat`

## Features
- **Hot Reload Support**  
  The project supports hot reloading, allowing changes to be instantly reflected after modification.  
  - **Solution Configuration**  
    The solution configuration is set to `FWDLL`. Build the project using this configuration for proper operation.
  - **Reload Method**  
    Press the `F5` key to enable hot reloading.
