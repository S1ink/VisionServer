# VisionServer
__VisionServer was originally a test project for learning how to implement vision on an FRC robot (raspberry pi coprocessor), but now has morphed into an api for multiple types of vision workflows. The base api features currently includes:__
  - Loading camera configurations and calibrations from a json (ex. /boot/frc.json on wpilibpi)
  - Handling any number of cameras (generated from json) and switching the output stream between each of the sources (networktable setting)
  - Brightness, exposure, and whitebalance control for each camera during runtime through networktables
  - Setting compression of the output stream (currently a compile-time only value -> this seems to be a wpilib problem)
  - An extendable pipeline interface for creating vision pipelines (and default pipeline that acts as a video passthrough)
  - Automatic networktables subtable creation for each pipeline, allowing any number of settings to be available for each
  - Switching between up to 3 pipelines during runtime through networktables
  - Thresholding and contour helper classes that can be implemented into pipelines
  - A target class that takes in the number of corners and world points of a target (can represent any target), implements solvePnP to find rotation and translation vectors, and updates networktables values accordingly (uses camera calibration values of current camera, which are from the json)
  - A sample "Target Solving" pipeline that takes in a target and thresholding class and can implement solvePnP for any mixture of the two
  - A networktable value representing which target is currently detected, with an automatic timeout for when none are (multiple targets could be implemented in a single pipeline)
  - Various statistics such as framerate, frametime, processing time, CPU usage and temperature, etc. available over networktables
  - Toggleable statistics on the output stream
  - Running a processing instance in a separate thread -> theoretically this would mean you could run multiple instances although this is not officially implemented yet

__User extendable features include:__
  - Custom pipelines
  - Custom targets
  - Custom thresholding techniques

  (Or just edit the api source and do anything you want!)
  
__Additionally, source code for interfaceing with VisionServer from a robot-side program is included for both Java and C++ under [_robot-impl_](robot-impl).__

# Getting Started
__To start using VisionServer, either download the sample program, the prebuild library and headers, or the entire project (recommended). The demo and library can be downloaded from the latest release, and the project can be either cloned or downloaded the same way.__

### **_IMPORTANT - READ BEFORE DOWNLOADING_**
- The library needs to be linked appropriately with OpenCV and other Wpilib libraries - please read the tutorial on building for further information.
- The project requires a cross-compiler and additional configuration to be developed and build properly on Windows - more info in the tutorial.
- Also note that target solving requires camera calibration values which need to be added to the source json. A sample json is included in the root of this repo, but more info can be found in the tutorial.

**_Also note that there is no tutorial yet... COMING SOON_**
