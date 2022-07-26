# Robot Project Integration
__Although it is not strictly necessary to develop vision code alongside robot code, combining the two into a single project can be helpful for cross-referencing during development, and compartmentalizing code for similar tasks. Additionally, this makes it easy to publish all season code, both for a robot and for vision, to a single repository.__

__This tutorial will traverse through the process of integrating the VisionServer library into a robot project.__

---
## Filestructure
1. __Create/have a WPILib robot project.__ More info can be found [here](https://docs.wpilib.org/en/stable/docs/zero-to-robot/step-4/creating-benchtop-test-program-cpp-java.html).
2. __Setup the vision program directories.__
	* In the base of your project, create a folder that will contain all vision code for the raspberry pi (ex. `vision/`).
	* Now navigate inside this folder, and create 3 folders: one for all source files, one for intermediate files, and one for output files (ex. `src/`, `obj/`, and `bin/`).
	* The filestructure should now resemble something like this:
		```
		> Robot-Project/
			> .gradle/
			> .vscode/
			> .wpilib/
			> gradle/
			> src/
			> vision/
				> bin/
				> obj/
				> src/
			> other build files...
		```

---
## Project Files
3. __Clone VisionServer.__ While still inside the base vision directory, open a terminal and enter `git clone https://github.com/FRC3407/VisionServer --recurse-submodules` to clone this repo and its submodules.
	* If your base project is already a git repo, and you want to add VisionServer as a submodule (definitely recommended for this config), then navigate to the base project folder and `git submodule add https://github.com/FRC3407/VisionServer VISION_PROGRAM_RELATIVE_PATH/VisionServer` - where for this tutorial `VISION_PROGRAM_RELATIVE_PATH` would be `vision` (as created in step 1).
	* The filestructure should now resemble something like this:
		```
		> Robot-Project/
			> ...
			> vision/
				> bin/
				> obj/
				> src/
				> VisionServer/
			> other build files...
		```
4. __Create source files (begin development).__ Within the `src/` subdirectory, create your source files. This step is intentionally vague because the this is not a programming guide.
	* *See the link at the end for an example project.*
5. __Setup the build system.__ This could involve make, cmake, or even gradle. I personally use a makefile similar to that used for building VS itself.
	* For linking to VisionServer, the shared and static library binaries can be found in `...VisionServer/lib-vs/out/`. Headers are also contained in the `include/` subdirectory, although it is more convenient to just include straight from the source folder instead (`...VisionServer/lib-vs/src/`).
	* [BUILD.md](BUILD.md) contains more info about linking to and building the library itself.
	* All libraries that VS depends on can also be conveniently utilized by a user-project (ex. OpenCV), and are located under `...VisionServer/lib-vs/lib/` and `...VisionServer/lib-vs/indlude/`.
	* *See the link at the end for an example project.*
6. __Add the robot-side API files into your robot project.__ Within the base project directory, navigate to where your source files are located (ex. `src/main/java/frc/robot/` for Java), and `git clone https://github.com/S1ink/VisionServer-Robot API_DIR` - where `API_DIR` represents the directory name where the repo will be cloned (ex. `vision` is once again a good name).
	* Again, if submodules are preferred, then this should be added as a submodule from the base project directory: `git submodule add https://github.com/S1ink/VisionServer-Robot SRC_DIR_PATH/API_DIR`.
	* Note that for Java projects the "package" line at the top of the added files will need to be updated based on the directory names used above.

---
## Additional Automations (Optional)
7. __Configure VSCode workspace and tasks.__ This is the same process as described in step 4 in [SETUP.md](SETUP.md), except that include paths may need to be adjusted based on the build system present. A build task can be constructed to invoke whatever build system is used with the correct parameters, and allows for a keybound build command.
8. __Rebuild VisionServer before each project build.__ If you plan on editing the library itself during development, you can simply create a build script that calls `make -C VISION_DIR/VisionServer/lib-vs shared OPT=release` or `make -C VISION_DIR/VisionServer/lib-vs static OPT=release` before the vision program itself is built. This will update `...VisionServer/lib-vs/out/libvs3407.so [and .a]` along with recopy all headers into `...VisionServer/lib-vs/out/include/`, which can then be used as artifacts for the program build.

---
## Example Project
__An example project used during the Rapid React FRC season can be found [here](https://github.com/S1ink/2022-Rapid-React).__