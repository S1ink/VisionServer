[![CI](https://github.com/FRC3407/VisionServer/actions/workflows/ci.yml/badge.svg)](https://github.com/FRC3407/VisionServer/actions/workflows/ci.yml) [![Build Docs](https://github.com/FRC3407/VisionServer/actions/workflows/doxygen-pages.yml/badge.svg?branch=main)](https://github.com/FRC3407/VisionServer/actions/workflows/doxygen-pages.yml)

# [FRC 3407] VisionServer
## Project Overview
 - This project originally existed as a testing environment for vision processing code that would be run on an FRC robot (Raspberry Pi coprocessor).
 - C++ was chosen as the primary development language, and a more generalized program framework was created, titled 'VisionServer'.
 - VisionServer[v1.*] was released as a framework which could be reimplemented by modifying the main source file(s) and recompiling the entire project. These releases were utilized during 2021 testing and the 2022 Rapid React season; provided features for both ease of use [on a robot] and advanced vision processing techniques.
 - VisionServer[v2.*+] now comes with a redesigned program (library) structure that focuses on concurrency and multithreading. The project is also now implemented as a library that allows easier integration in external projects, and can functionally be used as a git submodule. This repo contains automations for updating the necessary dependencies (wpilib, opencv, tflite), which makes creating a vision program simpler and more accessible.

__Highlighted Features:__
- Load and run unlimited* vision processing pipelines - although obviously limited by hardware specs
- Run each pipeline in its own thread for concurrent operation, or one at a time in singlethreaded mode
- Extendable pipeline class for easily running custom pipelines
- Parse ntable and camera settings from frc.json (WPILibPi config file) or any other provided json in order to auto-initialize cameras (and calibration data)
- Setup any number of output streams (amount limited by cscore) and dynamically assign input sources during runtime
- Chain pipeline outputs dynamically during runtime (although this is very non-performant)
- SequentialPipeline class for running any number of pipelines sequentially - this accomplishes the same as above but is much more performant
- Camera feeds can be processed by multiple pipelines concurrently without losing frames
- Automatic networktable integration for pipelines, output streams, and main settings for dynamic runtime control (and robot communication)
- Target class for sending target information to a robot over networktables
- TfLite libraries included, along with base pipeline classes to support training and using [WPILib] Axon object detection models
- EdgeTPU library included for running hardware accelerated TfLite models

__Simple object detection program using VisionServer (and an axon-generated tflite model):__
```cpp
#include <vector>
#include <core/visioncamera.h>
#include <core/visionserver2.h>
#include <core/vision.h>
#include <core/tfmodel.h>

using namespace vs2;

int main() {
    std::vector<VisionCamera> cameras;
    readConfig(cameras);            // default config is /boot/frc.json

    VisionServer::Init();           // verbosely initialize VS rather than allow lazy-loading
    VisionServer::addCameras(std::move(cameras));
    VisionServer::addStream("vision");
    AxonRunner a("model.tflite", TfModel::Optimization::EDGETPU, "map.pbtxt", 4);   // filename for model, attempt to load as edge tpu optimized, filename for labels, use 4 threads
    VisionServer::addPipeline(&a);  // or VisionServer::addPipeline<AxonRunner>(); for [default-constructed,] dynamically allocated pipeline
    VisionServer::run(50);          // the target (and maximum) fps

    atexit(VisionServer::stopExit);  // stop the server when the program ends
    return 0;
}
```

## Get Started
- __[Setup guide](SETUP.md) for basic developement requirements__
- __[A guide](ROBOT_PROJECT.md) on integrating a vision project into a WPILib robot project__
- __[More info](BUILD.md) on building the project, or building another project and linking to VS__
- __[Some helpful tips](WPILIBPI_TIPS.md) for deploying to and using WPILibPi (and specifically when attached to a robot)__
- __*Coming Soon* - a guide on using the included calibration tools, and how to included calibrations in the config json__
- __[Source code](/roborio-vs) for robot[program]-side networktables communication abstraction API - *At any given time this may not be completely up to date with the latest VS source/release*__

## Documentation
- __Doxygen-generated documentation is currently not working but can be found [here](https://frc3407.github.io/VisionServer/doxygen/html/).__

## More on Vision Processing
- __Some [helpful resources](REFERENCES.md) used in the making of this project__

## Suggestions?
__Currently [I](https://github.com/S1ink/) am the sole developer and maintainer of this project, so any suggestions/help are welcome (note that things don't get done very fast) - just post an issue or create a pull request. There is no template for this, just use common sense and try to be as helpful as possible.__