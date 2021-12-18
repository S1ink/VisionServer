#include "vision.h"

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

template<typename num_t>
void rescale(std::vector<cv::Point_<num_t> >& points, size_t scale) {
	for(size_t i = 0; i < points.size(); i++) {
		points[i] *= (int)scale;
	}
}
template<typename num_t>
void _rescale(std::vector<cv::Point_<num_t> >& points, size_t scale) {
	for(size_t i = 0; i < points.size(); i++) {
		points[i] /= (int)scale;
	}
}
template<typename num_t>
void rescale(std::vector<cv::Point_<num_t> >& points, double scale) {
	for(size_t i = 0; i < points.size(); i++) {
		points[i] *= scale;
	}
}

template<typename num_t>
cv::Size_<num_t> operator/(cv::Size_<num_t> input, size_t scale) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	return cv::Size_<num_t>(input.width/scale, input.height/scale);
}

template<typename num_t>
cv::Point_<num_t> findCenter(const std::vector<cv::Point_<num_t> >& contour) {
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
template<typename onum_t, typename inum_t>
cv::Point_<onum_t> findCenter(const std::vector<cv::Point_<inum_t> >& contour) {
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
template<typename num_t>
cv::Point3_<num_t> findCenter3D(const std::vector<cv::Point3_<num_t> >& contour) {
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
cv::Point3_<onum_t> findCenter3D(const std::vector<cv::Point3_<inum_t> >& contour) {
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
void reorderClockWise(std::vector<cv::Point_<num_t> >& points) {
	cv::Point_<num_t> center = findCenter<num_t, num_t>(points), abuff, bbuff;
	std::sort(
		points.begin(),
		points.end(),
		[center, &abuff, &bbuff](const cv::Point_<num_t>& a, const cv::Point_<num_t>& b) {
			abuff = a - center;
			bbuff = b - center;
			return -atan2(abuff.x, abuff.y) < -atan2(bbuff.x, bbuff.y);
		}
	);
}
template<typename num_t>
void reorderCClockWise(std::vector<cv::Point_<num_t> >& points) {
	cv::Point_<num_t> center = findCenter<num_t, num_t>(points), abuff, bbuff;
	std::sort(
		points.begin(),
		points.end(),
		[center, &abuff, &bbuff](const cv::Point_<num_t>& a, const cv::Point_<num_t>& b) {
			abuff = a - center;
			bbuff = b - center;
			return atan2(abuff.x, abuff.y) < atan2(bbuff.x, bbuff.y);
		}
	);
}