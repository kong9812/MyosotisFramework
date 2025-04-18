# <img src="https://github.com/user-attachments/assets/a8c13339-e5bf-4402-951f-7e1998412b93" width="30">MyosotisFramework<img src="https://github.com/user-attachments/assets/a8c13339-e5bf-4402-951f-7e1998412b93" width="30">
A framework created with Vulkan
![DEMO](https://github.com/user-attachments/assets/32ac263e-6dde-4ed3-b8a2-3e1ea7108016)

> [!IMPORTANT]
> This software is still under development and contains many unimplemented or partially implemented features.  
> Please be cautious when using it.  
> **Feel free to modify it as you wish.**

## Build status (AppVeyor)
| [Windows - Build status][win-link] |
| :--------------------------------: |
|           ![win-badge]             |

[win-badge]: https://ci.appveyor.com/api/projects/status/6tr51vx4415hflfo/branch/main?svg=true "AppVeyor build status"
[win-link]:  https://ci.appveyor.com/project/kong9812/myosotisframework/branch/main "AppVeyor build status"

---

## Clone Command
Use the following command to clone the repository.  
```git clone --recursive https://github.com/kong9812/MyosotisFramework.git```

---

## Requirements
- **[cmake](https://cmake.org)**
- **[Git](https://git-scm.com)**
- vswhere

---

## Environment Setup (Windows)
To set up the environment, use the provided batch file `envSetup.bat`.  
This file will:
1. Check if `git` and `cmake` are installed on your system.
2. Install them if they are not already installed.
3. Create any required junctions to ensure proper paths and dependencies are set up correctly.  

> [!NOTE]
> macOS and Linux support is included in the TODO list, but it will take time to make it happen…

## Build the Solution
Once the environment setup is complete, you can use the following batch file to create the solution file:  
`cmakeBuild.bat`

> [!NOTE]
> <details>
>   <summary>Build Instructions (in my environment)</summary>
> 
>   To build the project in my environment, follow these steps:
> 
>   1. Run `envSetup.bat` to set up the environment.
>   2. Run `cmakeBuild.bat` to configure the project with CMake.
>   3. Run `buildAllShaders.bat` to build all the shaders.
>   4. Open the solution by double-clicking `bin/MyosotisFW.sln` (Visual Studio 2022).
>   5. In Solution Explorer, set `MyosotisFW` as the startup project.
>   6. Choose the solution configuration (Debug/FWDLL/Release) and build & run the project.
> 
>   These steps are just what works for me. If you have any issues, feel free to reach out!
> </details>

---

## Features
- **Hot Reload Support**  
  The project supports hot reloading, allowing changes to be instantly applied after modification.   
  - **Solution Configuration**  
    The solution configuration is set to `FWDLL`. Build the project using this configuration to ensure proper operation.
  - **Reload Method**  
    Press the `F5 key` to trigger hot reloading.

- **ImGui**  
  ImGui has been integrated into the project, providing an easy-to-use graphical user interface for debugging and development.

- **LOD Functionality**  
  Definition of LOD distances (Very Close, Close, Far)  
  Objects beyond the "Far" distance become hidden.

- **Pause Functionality (In Progress)**  
  Press the `F2 key` to pause the update process.

- **Frustum Culling**

- **Deferred Rending**

- **Interior Mapping**  
![interiorMapping](https://github.com/kong9812/MyosotisFramework/blob/main/demo/interiorMapping.gif)

---

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
> [!IMPORTANT]
> ## Disclaimer
> This software is provided "as is", without any express or implied warranties.  
> In no event shall the authors or copyright holders be liable for any loss or damages arising from the use of this program.  
> The authors do not take responsibility for any consequences or losses that may result from using this software.

## Credits and Attributions
See [THANKS.md](THANKS.md) for additional credits and attributions.
