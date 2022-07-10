# Using WPILibPi
__Before reading this guide, make sure to familiarize yourself with WPILibPi using [its own docs](https://docs.wpilib.org/en/stable/docs/software/vision-processing/wpilibpi/).__
## Shared Libraries - IMPORTANT
WPILibPi comes preinstalled with WPILib, OpenCV, and Pigpio, but not with Tensorflow Lite (only the python version is present, not C++) or the Edge TPU driver - these will have to be uploaded manually. Both files (libtensorflowlite.so and libedgetpu.so) are copied to the output directory during each VS build, and can be found there for uploading on the Pi using the web dashboard (these are treated as "supplementary files" in step 2 below). Uploading these should be a one-time process, but will need to be repeated when newer versions of WPILibPi are released, and this repo is updated with newer versions of TfLite and the EdgeTPU driver.
## Deployment Process
1. __Build your project (and/or VS).__
2. __Go to the WPILibPi dashboard (http://wpilibpi - or http://IP_ADDRESS_OF_RPI) in your browser and navigate to the `Application` tab.__ Make sure to toggle the Pi into __Writable__ mode using the button at the top of the page.
	* To upload a new vision program, select __"Uploaded C++ executable"__ from the dropdown, then select the binary from the output directory and upload it.
	* To upload a new version of the VisionServer shared library (libvs3407.so), or other supplementary files (ex. a tflite model), navigate towards the bottom of the page under __"Upload other files"__ and select the required files. Note that in the case of the shared library, the old version within the Pi's filesystem will simply be replaced with the newer version provided that they are named the same.
	* Change the Pi back to __Read-Only__ mode before proceeding.
3. __Run the program.__ This can be done through the web dashboard or by ssh'ing into the Pi and running the program manually.
	* On the dashboard, navigate to the `Vision Status` tab and select the __"Up"__ option to start the program. Additionally make sure to enable console output, which is oftentimes crucial for debugging.
	* When ssh'ed into the Pi, enter `sudo ./uploaded` to run the program. This can be helpful when more direct terminal output is needed (sometimes the dashboard can be misformatted and skip some lines).
## "Patches"
For some reason uploaded executables may require sudo permissions to work properly, and without can sometimes fail to output streams and simply not work correctly. By default, WPILibPi runs uploaded executables without sudo, but this can be changed with a few modifications.
1. __SSH into your Pi.__ Additionally go to the web dashboard and set the Pi to __Writable__ mode.
2. __Enter `sudo nano /etc/service/camera/run` to begin editing this file.__
3. __Change `exec pgrphack /usr/local/bin/setuidgids pi ./runCamera` to `exec pgrphack ./runCamera`.__
4. __Use `Ctrl + S` to save, then `Ctrl + X` to exit.__
5. __Set the Pi back to __Read-Only__ and run the program through the dashboard - it should work normally.__
## Troubleshooting (Robot Deployment)
Using and connecting to the Pi directly may seem fluid and stable, but there are a whole host of issues that can arrise during robot deployment. This can include anything from short stutters/freezing video streams, to unresponsive connections and a complete inability to connect.

 * Make sure the Pi is in __"Client"__ mode when connected to a robot (changed in the `Network Settings` tab). This can be a very difficult issue to diagnose due to the fact that sometimes NetworkTables will work correctly and other times it will not. Additionally, this can be quite common if server mode is used for testing the Pi while not operating on a robot.
 * Make sure that the radio power is secure. Sometimes the wires can be poorly connected to the VRM and cause short intermittances. This will not affect robot control, but does cause a much longer delay (freeze) in all RPi communications. This problem can be diagnosed if the ethernet LEDs on the Pi go dark during operation, as this means that communication has been lost between the radio and Pi.
 * Make sure that the ethernet cable is sequrely connected. This can be a similar issue to the previous, where either a bad cable or loose plug can cause intermittant disconnections.
 * Make sure to update NetworkTable dashboard settings from testing. Oftentimes these will be set to connect directly to the Pi for testing, but must be reset during robot control so that they connect to the RoboRIO.