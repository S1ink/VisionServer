# VisionServer
__VisionServer was originally a test project for learning how to implement vision on an FRC robot (raspberry pi coprocessor), but now has morphed into an api for multiple types of vision workflows. The base api currently features:__
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
__A guide for downloading and setting up the project can be found [here](SETUP.md). Additionally, a precompiled library (and headers) of the api along with a demo program can be found in the latest [release](https://github.com/FRC3407/VisionServer/releases). If you use the library in your own project, make sure to link in the libraries for OpenCV and Wpilib correctly (these are found in [lib/](../lib/) and [include/](../include/), link options can be found in the [makefile](../makefile)).__
  - *Note that the demo program is configured to track pure blue (the color or the led I am testing with), and target solving requires camera configuration values to be present within the source json (see the included [json](../frc.json)).*
