# Setup
#### __Although the project has only ever been developed on Windows, cross platform support should be possible using VSCode (which is cross-platform) and cross-comipler builds from the wpilib raspbian-toolchain repository, which both have downloads for Windows, MacOS and Linux. Simply follow the steps below to begin developing your own vision code for a raspberry pi coprocessor.__
## Dependencies/Environment
1. __Install VSCode.__ This can either be downloaded directly from [it's website](https://code.visualstudio.com/) or by using the [WPILIB Installer](https://docs.wpilib.org/en/stable/docs/zero-to-robot/step-2/wpilib-setup.html) (which you probably already have if you are developing for FRC). Make sure to download the correct version for your platform and architecture from either source. 
    - Note that you will also want to have the C/C++ extension installed for VSC. This comes already with WPILIB if you clicked `Visual Studio Code Extensions`, but can be installed manually by clicking on the extensions tab (the side bar) in VSC, and then searching for `C/C++ Extension Pack`. [(this one)](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack)
2. __Install the cross-compiler.__ This is avaiable at the wpilib [raspbian-toolchain repository](https://github.com/wpilibsuite/raspbian-toolchain) for each platform under [releases](https://github.com/wpilibsuite/raspbian-toolchain/releases). At the time of writing, the latest verions is 3.0.0, but you should download from the most up to date release. 
    - Now extract the downloaded archive and place the output folder wherever you see fit (ex. `C:\Program Files` on Windows), or just leave it in your downloads folder. In either case, take note of the location as this path will be needed later. 
    - For Windows, an executable of Make can also be found in the toolchain repository under [tools](https://github.com/wpilibsuite/raspbian-toolchain/tree/main/tools) (and was included directly in this repo in older versions). Download this if you plan to use the preexisting makefile for the project, which is recommended. 
      - *This exact same file can be found in the wpilib install (if you have it installed) under `C:\Users\Public\wpilib\YEAR\roborio\bin` and titled `frcYEAR-make.exe`. This should work the same as the one from the repo, although you might want to copy and rename it.*
    - Add the cross-compiler "bin" folder to your PATH environment variable. The process will be different for each platform, but a simple google search should show how (MacOS and Linux). On Windows, [this guide](https://www.architectryan.com/2018/03/17/add-to-the-path-on-windows-10/) can be followed. 
      - *The "bin" folder should be `(location from first bullet point)/Raspbian10-Windows64-Toolchain-8.3.0/raspbian10/bin` (or a similar structure for difference versions and platforms) and should contain quite a few executables.*
      - *I also copy/move the __Make executable__ (see the previous bullet point) to this directory so it is accessable along with the other programs.*
      - *As an example, my PATH looks like this, with the highlighted entry being the bin folder and my location folder being `D:\Programs`.*

        ![image](https://user-images.githubusercontent.com/60528506/147836117-fec75d6f-2871-46fa-9e10-3c21a7b73024.png)
## Configuring the Project
3. __If you haven't already, download the project by cloning directly from the [latest commit](https://github.com/FRC3407/VisionServer) or by downloading the source code archive from a [release](https://github.com/FRC3407/VisionServer/releases).__
    - The project should be able to be built right away by running `make` in the command line in the project directory (try it to test that everything is working), but additional configuration needs to be done to setup a VSCode workflow. 
4. __Open the project in VSCode. In order for syntax highlighting and intellisense to function, the file `(project root)/.vscode/c_cpp_properties.json` needs to be updated with the location of your cross-compiler, the correct "intelliSenseMode", STL version, include paths, and predefined macros. By default, the file should contain 4 separeate configurations, the first one being the default configuration (commented out) and the other 3 being machines that I use. Copy one of the configuration objects (they start and end with '{' and '}') and paste it anywhere within the `"configurations" : [ (here) ]` block (you can now delete all the others). Now edit each variable like specified below. *Please note that the format of this file could change with VSCode updates in the future, so the exact entry names may change. Refer to [this](https://code.visualstudio.com/docs/cpp/c-cpp-properties-schema-reference) for updated info on formatting.*__ 
    - Change the `"name"` variable to something that represents your machine and other settings in the configuration (ex. CC-ARM-{COMPUTER NAME}). 
    - Add any aditional folders that you want to include under `"includePath"`. If you add anything, make sure to update the makefile accordingly so that the folders are actually included during a compile (see the [makefile walkthrough]() for more info). It is recommended to leave the preexisting values (as shown below) untouched as they reflect the default includes of the makefile (crucial to the build).
      ```
      "${workspaceFolder}/**",        // <- the root directory
      "${workspaceFolder}/include",   // <- all OpenCV and WPILIB headers
      "${workspaceFolder}/references" // <- used for accessing the sources under "tools/src"
      ```
    - Add any predefined macros that you add to the makefile under `"defines"`. 
    - `"compilerPath"` should be the full path to g++, which is inside the "bin" folder which was added to path in the previous steps. Simply go to the folder and copy the path, then append the executable name to the end (something like `arm-raspbian10-linux-gnueabihf-g++.exe`). 
    - You can leave `"cStandard"` and `"cppStandard"` alone unless you know for sure that a different version is being used (newer cross-compiler build). 
    - Make sure `"intelliSenseMode"` is set to `"linux-gcc-arm"` or the equivelant if the preset names change due to an update. 

    Now the file should look something like this:
    ```
    {
      "configurations": [
        {
          "name": YOUR CONFIGURATION NAME,
          "includePath": [
            "${workspaceFolder}/**",
            "${workspaceFolder}/include",
            "${workspaceFolder}/references",
            OTHER INCLUDE LOCATIONS...
          ],
          "defines": [
            "UNICODE",
            "_UNICODE",
            "__linux__",
            "__unix__",
            OTHER DEFINES...
          ],
          "compilerPath": "PATH TO YOUR CROSS COMPILER/Raspbian10-Windows64-Toolchain-8.3.0\\raspbian10\\bin\\arm-raspbian10-linux-gnueabihf-g++.exe",
          "cStandard": c17,
          "cppStandard": c++17,
          "intelliSenseMode": "linux-gcc-arm"
        }
      ]
    }
    ```
    
    After everything is edited, save the file and make sure VSCode is using the correct configuration. Open a C/C++ file and check to see that the text in the bottom-right corner matches the name specified in the configuration like shown below. 
    - If it says something different, click on the text and then select the correct configuration from the dropdown menu. 
    
    ![image](https://user-images.githubusercontent.com/60528506/147838053-0760cef5-66f0-46e1-a907-cb59fa9349bd.png)

__After that, everything should be good to go! You can use `Ctrl + Shift + b` to start a build task, which by default calls `./make` in a terminal instance. To change build tasks, you can edit `(project root)/.vscode/tasks.json`.__

*Note that there are included project files for Visual Studio (not VSCode), although the integrated build system is messed up. If you would like to develop the project with Visual Studio, you will have to configure the project properties for your cross-compiler path (for syntax highlighting) and build using the terminal.*

For info on running the program or building natively on a rpi, see the [raspberry pi documentation](). 
