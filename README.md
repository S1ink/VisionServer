# 2022-VisionTesting
__This repo contains code for the different iterations of testing of vision processing code during the 2021-2022 robotics season (FRC). Currently this project is designed for use
on a raspberry pi coprocessor, located on the robot and connected to the robot network.__
- Instructions on setting up the raspberry pi with WpiLibPi can be found [here](https://docs.wpilib.org/en/stable/docs/software/vision-processing/wpilibpi/what-you-need-to-get-the-pi-image-running.html). (Keep following the guide for info on the web interface)
- Example projects that were used as a base for these are located in the wpilibpi repository - [source](https://github.com/wpilibsuite/WPILibPi/tree/main/deps/examples), 
[downloads](https://github.com/wpilibsuite/WPILibPi/releases).
## Branches
Currently only a C++ branch exists, but in the future there may be others based on which additional languages are tried for vision. The 'release' branch is for production level 
code or the project with the most features/functionality. This will be utilized when more than one language is active. 
# C++ Project (Version) Info
- Source files are located in [*src/*](src)
- Additional includes are located in [*include/*](include)
- Libraries are located in [*lib/*](lib)
- Intermediate files (.o and .d) files are placed in [*obj/*](obj) and the output binary(s) are located in [*bin/*](bin)
- General resources of sources from example projects are placed in [*resources/*](resources)

  ## Setup/Workflow (and external dependencies)
  - ***Note: Read the entire process before attempting to build (some dependencies may need to be installed from other sources)***
  
  The project is built with the included [__makefile__](makefile). An executable of Make is included for windows, (this was copied from an example project from wpilib) but the 
  project can also be build natively on a raspberry pi. To build, simply run the command `make` within the project directory. By default the makefile is setup to build in release
  mode (-O3, no debugging), and for windows (appends the cross-compiler prefix to the build command). This can be modified by either specifing command line arguments or changing
  the makefile directly (the '*mode*' variable at the very top). Options are listed below:
  - `windows-release` (default option, compiler options already stated)
  - `windows-debug` (-g -Og, cross-compiler prefix)
  - `native-release` (-O3, no cross-compiler prefix)
  - `native-debug` (-g -Og, no cross-compiler prefix)
  
  For example, in the command line, type `make mode=native-release`. To change the default, edit the makefile and change `mode ?= windows-release` to `mode ?= {option}`.
  - ***Note: Other settings can obviously be changed but these will not be discussed - see https://www.gnu.org/software/make/manual/html_node/index.html for info on makefiles.***
  
  On Windows (and MacOS), the cross-compiler (for raspberry pi) will need to be installed - this can be found on the 
  [wpilib rpi toolchain](https://github.com/wpilibsuite/raspbian-toolchain/releases) repository. 
  
  For windows, once downloaded and extracted, either create a shortcut to the compiler (*EXTRACTED_DIR/raspbian10/bin/arm-raspbian10-linux-gnueabihf-g++.exe*) and place it in 
  the project folder, or add the folder containing the compiler to your PATH. (Directions [here](https://www.architectryan.com/2018/03/17/add-to-the-path-on-windows-10/) - 
  the directory should be *...EXTRACTED_DIR/raspbian10/bin/* like mentioned above)
  
  Additionally, project files for both VS Code and Visual Studio are included.

  | IDE | Intellisense working? | Integrated "build command" working? | 
  |:-:|:-:|:-:|
  | VS Code | Yes - Intellisense seemed to work fine but it is notorious for being unreliable for C++. Make sure all the correct extentions are intalled and VS Code is not getting confused with a compiler elsewhere on your system. | Yes - [*tasks*](https://code.visualstudio.com/docs/editor/tasks#vscode) can be added to [./vscode/tasks.json](./vscode/tasks.json) | 
  | Visual Studio | Yes - Make sure to set the correct include, library, and standard library paths within the project properties. The include and library paths should work as-is as they use relative paths, but the standard library directory will need to be added specific to each machine. (The options is under "VC++ Directories" and called "External Include Directories" - the path should be *...EXTRACTED_DIR/raspbian10/sys-root/usr/include/* from the downloaded cross-compiler) | No - For some reason VS always generates an error even though it seems to run the correct command. A simple, but more annoying solution is to open a terminal within the application and just enter the build command that way. |
  
  Once the build process is working correctly, it is now possible to test a vision program on the rpi.
  - On Windows, build the project using the methods described above, then find the output binary in the *bin/* directory. Head over to the WpiLibPi dashboard (make sure your networking setup is correct and you can access the rpi) and go to the "Application" tab. Select "Uploaded C++ Executable" from the options and then navigate to the directory with your program. Select the program and then click "Save" (make sure the rpi is in writable mode). Now you can simply run the program from the "Vision Status" tab, and see its output in the designated space. 
  - On the rpi, transfer your project directory so that it is accessable on the rpi filesystem, either by a removable storage device or a network share. Gain access to the cli either by connecting a moniter and keyboard to the pi or ssh'ing into it with the same hostname that is used for the web dashboard. Navigate to the project directory and run the `make` command (make sure to use `mode=native-{option}` to build natively) to build the project if it wasn't alread built. The program can either be run directly in the build folder or can be copied to the home directory for execution from the web dashboard. Enter `sudo ./{program_name}` to run the program directly, and enter `cp {program_name} /home/pi` to copy the program (make sure you have `cd`'ed to the *bin/* directory). If copying to the home directory, the "runCamera" shell file will also need to be modified to point to your program. Enter `sudo nano runCamera` from the home directory and then edit it like shown below: 
    ```
    #!/bin/sh
    echo "Waiting 5 seconds..."
    sleep 5
    exec ./{program_name}
    ``` 
    The "Application" setting on the web dashboard may also need to be changed to "C++ Executable" for the correct program to be run. 
  - See [this](https://docs.wpilib.org/en/stable/docs/software/vision-processing/wpilibpi/the-raspberry-pi-frc-console.html#application) for more info on vision applications and WpiLibPi. 
    
  ## Other Notes
  - The headers within *include/* and libraries within *lib/* originate from the C++ example project that can be downloaded from the WpiLibPi web dashboard (in the "Application" tab, under "Example Programs" -> [image](https://docs.wpilib.org/en/stable/docs/software/vision-processing/wpilibpi/the-raspberry-pi-frc-console.html#vision-workflows)). Currently I am unaware of a separate source to get these, so downloading them from the dashboard remains the best option to get the most up-to-date libraries. 
  - The [pigpio](https://abyz.me.uk/rpi/pigpio/) library is also included in the project. To install this manually, download and build pigpio, then copy *pigpio.h* into *include/* and *libpigpio.so* into *lib/*. There are a number of other libraries (.so files) that get made when pigpio is built, but none of these are needed. 
  ## A Quick Demo (Release V3)
  https://user-images.githubusercontent.com/60528506/144782431-3216b8dc-1f97-4670-a13f-1772aa9ad199.mp4
  
***This guide is not finished - more info to come***
