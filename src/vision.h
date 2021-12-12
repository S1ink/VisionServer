#pragma once

#include <networktables/NetworkTable.h>
#include <wpi/Twine.h>

#include <cameraserver/CameraServer.h>

#include <opencv2/core/types.hpp>

inline cv::Size getResolution(cs::VideoMode vm) {
    return cv::Size(vm.height, vm.width);
}

template<typename num_t>
cv::Size_<num_t> operator/(cv::Size_<num_t> input, size_t scale) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	return cv::Size_<num_t>(input.width/scale, input.height/scale);
}
// template<typename num_t>
// cv::Size_<num_t> operator*(cv::Size_<num_t> input, size_t scale) {
// 	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
// 	return cv::Size_<num_t>(input.width*scale, input.height*scale);
// }

template<typename num_t>
cv::Point_<num_t> findCenter(const std::vector<cv::Point_<num_t>>& contour) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	if(contour.size() > 0) {
		num_t x = 0, y = 0;
		for(size_t i = 0; i < contour.size(); i++) {
			x += contour[i].x;
			y += contour[i].y;
		}
		x /= contour.size();
		y /= contour.size();

		return cv::Point_<num_t>(x, y);
	}
	return cv::Point_<num_t>();
}
template<typename num_t>
cv::Point3_<num_t> findCenter3D(const std::vector<cv::Point3_<num_t>>& contour) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	if(contour.size() > 0) {
		num_t x = 0, y = 0, z = 0;
		for(size_t i = 0; i < contour.size(); i++) {
			x += contour[i].x;
			y += contour[i].y;
			z += contour[i].z;
		}
		x /= contour.size();
		y /= contour.size();
		z /= contour.size();

		return cv::Point3_<num_t>(x, y, z);
	}
	return cv::Point3_<num_t>();
}
template<typename onum_t, typename inum_t>
cv::Point_<onum_t> findCenter(const std::vector<cv::Point_<inum_t>>& contour) {
	static_assert(std::is_arithmetic<onum_t>::value, "Template parameter (num_t) must be arithemetic type");
	static_assert(std::is_arithmetic<inum_t>::value, "Template parameter (num_t) must be arithemetic type");
	if(contour.size() > 0) {
		onum_t x = 0, y = 0;
		for(size_t i = 0; i < contour.size(); i++) {
			x += contour[i].x;
			y += contour[i].y;
		}
		x /= contour.size();
		y /= contour.size();

		return cv::Point_<onum_t>(x, y);
	}
	return cv::Point_<onum_t>();
}
template<typename onum_t, typename inum_t>
cv::Point3_<onum_t> findCenter3D(const std::vector<cv::Point3_<inum_t>>& contour) {
	static_assert(std::is_arithmetic<onum_t>::value, "Template parameter (num_t) must be arithemetic type");
	static_assert(std::is_arithmetic<inum_t>::value, "Template parameter (num_t) must be arithemetic type");
	if(contour.size() > 0) {
		onum_t x = 0, y = 0, z = 0;
		for(size_t i = 0; i < contour.size(); i++) {
			x += contour[i].x;
			y += contour[i].y;
			z += contour[i].z;
		}
		x /= contour.size();
		y /= contour.size();
		z /= contour.size();

		return cv::Point3_<onum_t>(x, y, z);
	}
	return cv::Point3_<onum_t>();
}

template<typename num_t>
void addNetTableVar(num_t& var, const wpi::Twine& name, std::shared_ptr<nt::NetworkTable> table) {
	static_assert(std::is_arithmetic<num_t>::value, "num_t must be a numeric type");
	if(!table->ContainsKey(name)) {
		table->PutNumber(name.str(), var);
	} else {}
	table->GetEntry(name).AddListener(
		[&var](const nt::EntryNotification& event){
			if(event.value->IsDouble()) {
				var = event.value->GetDouble();
				//std::cout << "Networktable var(num) updated to : " << var << newline;
			}
		}, 
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}
void addNetTableVar(bool& var, const wpi::Twine& name, std::shared_ptr<nt::NetworkTable> table);

// cs::CvSink switchedCameraVision(
// 	const std::vector<VisionCamera>& cameras, 
// 	std::shared_ptr<nt::NetworkTable> table = nt::NetworkTableInstance::GetDefault().GetTable("Vision")
// ) {
// 	if(!table->ContainsKey("Vision Camera Index")) {
// 		table->PutNumber("Vision Camera Index", 0);
// 	}
// 	if(!table->ContainsKey("Vision Cameras Available")) {
// 		table->PutNumber("Vision Cameras Available", cameras.size());
// 	}
// 	static cs::CvSink source;
// 	if(cameras.size() > 0) {
// 		source = cameras[0].getVideo();
// 	}
// 	table->GetEntry("Vision Camera Index").AddListener(
// 		[&cameras](const nt::EntryNotification& event) {
// 			if(event.value->IsDouble()) {
// 				size_t idx = event.value->GetDouble();
// 				if(idx >= 0 && idx < cameras.size()) {
// 					source.SetSource(cameras[idx]);
// 					source.SetConfigJson(cameras[idx].getStreamJson());
// 				}
// 			}
// 		},
// 		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
// 	);
// 	return source;
// } 