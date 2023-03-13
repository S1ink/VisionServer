SOURCE: https://github.com/ligerbots/VisionServer/blob/master/utils/camera_calibration.py
REPO: https://github.com/ligerbots/VisionServer

1. Install python (link provided) - make sure pip is installed as an option in the installer

2. Open a terminal and install OpenCV along with Numpy
 - Run "pip install opencv-contrib-python"
 - Run "pip install numpy"
   >> If these don't work, make sure to add the folder which contains the python executable to your PATH

3. Run the cmd file or open a terminal and run the python file with different cli options (ex. "python camera_calibration.py {OPTIONS...}")
   >> make sure there are only images from a single type of camera in the "src" directory, or the calculations will not be correct
   >> also make sure to save/copy the json out of the "output" dir before calibrating another camera, or else this will be overwritten