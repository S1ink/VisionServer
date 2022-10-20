# Including Camera Calibrations
__This guide does not contain info for actually calibrating cameras [yet?]. A python script and some other references can be found in the source repository under `frc-config/calibration/`; otherwise consult OpenCV's docs.__


### The Old System:
~~~{.json}
##/boot/frc.json
{
   "team": <team number>,
   "ntmode": <"client", "server", or IP address of server(sim mode), "client" if unspecified>,
   "cameras": [
       {
           "name": <camera name>
           "path": <path, e.g. "/dev/video0">
           "pixel format": <"MJPEG", "YUYV", etc>   //optional
           "width": <video mode width>              //optional
           "height": <video mode height>            //optional
           "fps": <video mode fps>                  //optional
           "brightness": <percentage brightness>    //optional
           "white balance": <"auto", "hold", value> //optional
           "exposure": <"auto", "hold", value>      //optional
           "properties": [                          //optional
               {
                   "name": <property name>
                   "value": <property value>
               }
           ],
           "stream": {                              //optional
               "properties": [
                   {
                       "name": <stream property name>
                       "value": <stream property value>
                   }
               ]
           },
           "calibration": <name of calibration block(defined below)>
       }
   ],
   "switched cameras": [
       {
           "name": <virtual camera name>
           "key": <network table key used for selection>
           // if NT value is a string, it's treated as aname
           // if NT value is a double, it's treated as aninteger index
       }
   ],
   "calibrations": {
       <calibration name>: {
           "camera_matrix": [ [x3],[x3],[x3] ],
           "distortion": [ [x5] ]
       }
   }
}
~~~
Note the `calibrations` block, and how each calibration is referenced in the camera definitions above. In the program, simply calling `readconfig(~std::vector<VisionCamera>~)` will evaluate the config references and load each camera with the appropriate matrix/dist values if applicable.

---
### The New System:
(although also backwards compatible)
~~~{.json}
##/boot/frc.json
{
   "team": <team number>,
   "ntmode": <"client", "server", or IP address of server(sim mode), "client" if unspecified>,
   "cameras": [
       {
           "name": <camera name>
           "path": <path, e.g. "/dev/video0">
           "pixel format": <"MJPEG", "YUYV", etc>   //optional
           "width": <video mode width>              //optional
           "height": <video mode height>            //optional
           "fps": <video mode fps>                  //optional
           "brightness": <percentage brightness>    //optional
           "white balance": <"auto", "hold", value> //optional
           "exposure": <"auto", "hold", value>      //optional
           "properties": [                          //optional
               {	// for any other property
                   "name": <property name>
                   "value": <property value>
               },
			   {	// for calibration type property
					"type": <calibration type - defined in user code>
			   }
           ],
           "stream": {                              //optional
               "properties": [
                   {
                       "name": <stream property name>
                       "value": <stream property value>
                   }
               ]
           }
       }
   ],
   "switched cameras": [
       {
           "name": <virtual camera name>
           "key": <network table key used for selection>
           // if NT value is a string, it's treated as a name
           // if NT value is a double, it's treated as an integer index
       }
   ]
}
~~~
Here the main addition is the `cameras[...].properties[...].type` parameter. This should correspond to a string or indexable value that links to a key for a calibration map structure that is defined in user code (ex. "lifecam_hd3000"). There is no need to add a frame size specifier (required in old system) as this is automatically deduced based on the camera's size paramter. Although in the template json above the `calibrations` block was removed, the old style can still be utilized.

In code, include `<core/calib.h>` to create a static map of type-size-calibration data. Example usage:
~~~{.cpp}
##main.cpp
#include <core/calib.h>	// check out this file for "CalibList" definition

static const inline CalibList
	calibrations{
		{
			{ "lifecam_hd3000", {
				{ cv::Size{640, 480}, {
					cv::Mat1f{{3, 3},{
						673.6653136395231, 0, 339.861572657799,
						0, 666.1104961259615, 244.21065776461745,
						0, 0, 1}},
					cv::Mat1f{{1, 5},{
						0.04009256446529976, -0.4529245799337021,
						-0.001655316303789686, -0.00019284071985319236,
						0.5736326357832554}}
				} }
			} },
			{ "logitech_b910", {
				{ cv::Size{640, 480}, {
					cv::Mat1f{
						561.4074313049189, 0, 337.76418985338495,
						0, 557.6888346113145, 237.32558163183825,
						0, 0, 1},
					cv::Mat1f{
						0.03335218620375778, -0.42937056677801017,
						-0.001215946872533738, 0.011464765688132103,
						1.0705120077832195}
				} }
			} }
		}
	}
;

int main(int argc, char** argv) {
	std::vector<VisionCamera> cameras;

	if(argc > 1 && initNT(argv[1]) && createCameras(cameras, calibrations, argv[1])) {}
	else if(initNT() && createCameras(cameras, calibrations)) {}
	else { return EXIT_FAILURE; }

	//	...continue program...

}
~~~
Note the replaced call to `readConfig(...)` with a call to `initNT(~config_file~)` and a separate call to `createCameras(~std::vector<VisionCamera>~, ~CalibList~, ~config_file~)`, where you now must pass in the map of configs along with an uninitialized list of cameras.