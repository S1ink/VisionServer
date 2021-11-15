# 2022-VisionTesting
__This repo contains code for the different iterations of testing of vision processing code during the 2021-2022 robotics season (FRC). Currently this project is designed for use
on a raspberry pi coprocessor, located on the robot and connected to the robot network.__
- Instructions on setting up the raspberry pi with WpiLibPi can be found [here](https://docs.wpilib.org/en/stable/docs/software/vision-processing/wpilibpi/what-you-need-to-get-the-pi-image-running.html). (Keep following the guide for info on the web interface)
- Example projects that were used as a base for these are located in the wpilibpi repository - [source](https://github.com/wpilibsuite/WPILibPi/tree/main/deps/examples), 
[downloads](https://github.com/wpilibsuite/WPILibPi/releases).
## Branches
Currently only a C++ branch exists, but in the future there may be others based on which additional languages are tried for vision. The 'release' branch is for production level 
code or the project with the most features/functionality. This will be utilized when more than one language is active. 
## C++ Project (Version) Info
- Source files are located in [*src/*](src)
- Additional includes are located in [*include/*](include)
- Libraries are located in [*lib/*](lib)
- Intermediate files (.o and .d) files are placed in [*obj/*](obj) and the output binary(s) are located in [*bin/*](bin)
- General resources of sources from example projects are placed in [*resources/*](resources)

  ### Setup/Workflow (and external dependencies)
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
  
  ***This guide is not finished - more info to come***
