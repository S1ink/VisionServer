#include "vision.h"

template<typename num_t>
void addNetTableVar(num_t& var, const wpi::Twine& name, std::shared_ptr<nt::NetworkTable> table) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
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
cv::Point_<num_t> findCenter(const std::vector<cv::Point_<num_t> >& contour) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	if(contour.size() > 0) {
		cv::Point_<num_t> ret;
		for(size_t i = 0; i < contour.size(); i++) {
			ret.x += contour[i].x;
			ret.y += contour[i].y;
		}
		ret.x /= contour.size();
		ret.y /= contour.size();
		return ret;
	}
	return cv::Point_<num_t>();
}
template<typename onum_t, typename inum_t>
cv::Point_<onum_t> findCenter(const std::vector<cv::Point_<inum_t> >& contour) {
	static_assert(std::is_arithmetic<onum_t>::value, "Template parameter (num_t) must be arithemetic type");
	static_assert(std::is_arithmetic<inum_t>::value, "Template parameter (num_t) must be arithemetic type");
	if(contour.size() > 0) {
		cv::Point_<onum_t> ret;
		for(size_t i = 0; i < contour.size(); i++) {
			ret.x += contour[i].x;
			ret.y += contour[i].y;
		}
		ret.x /= contour.size();
		ret.y /= contour.size();
		return ret;
	}
	return cv::Point_<onum_t>();
}
template<typename num_t>
cv::Point3_<num_t> findCenter3D(const std::vector<cv::Point3_<num_t> >& contour) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	if(contour.size() > 0) {
		cv::Point3_<num_t> ret;
		for(size_t i = 0; i < contour.size(); i++) {
			ret.x += contour[i].x;
			ret.y += contour[i].y;
			ret.z += contour[i].z;
		}
		ret.x /= contour.size();
		ret.y /= contour.size();
		ret.z /= contour.size();
		return ret;
	}
	return cv::Point3_<num_t>();
}
template<typename onum_t, typename inum_t>
cv::Point3_<onum_t> findCenter3D(const std::vector<cv::Point3_<inum_t> >& contour) {
	static_assert(std::is_arithmetic<onum_t>::value, "Template parameter (num_t) must be arithemetic type");
	static_assert(std::is_arithmetic<inum_t>::value, "Template parameter (num_t) must be arithemetic type");
	if(contour.size() > 0) {
		cv::Point3_<onum_t> ret;
		for(size_t i = 0; i < contour.size(); i++) {
			ret.x += contour[i].x;
			ret.y += contour[i].y;
			ret.z += contour[i].z;
		}
		ret.x /= contour.size();
		ret.y /= contour.size();
		ret.z /= contour.size();
		return ret;
	}
	return cv::Point3_<onum_t>();
}

template<typename num_t>
cv::Point_<num_t> findCenter(const cv::Rect_<num_t>& rect) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	cv::Point_<num_t> ret;
	ret.x = rect.tl().x + rect.br().x;
	ret.y = rect.tl().y + rect.br().y;
	ret.x /= 2;
	ret.y /= 2;
	return ret;
}
template<typename onum_t, typename inum_t>
cv::Point_<onum_t> findCenter(const cv::Rect_<inum_t>& rect) {
	static_assert(std::is_arithmetic<onum_t>::value, "Template parameter (num_t) must be arithemetic type");
	static_assert(std::is_arithmetic<inum_t>::value, "Template parameter (num_t) must be arithemetic type");
	cv::Point_<onum_t> ret;
	ret.x = rect.tl().x + rect.br().x;
	ret.y = rect.tl().y + rect.br().y;
	ret.x /= 2;
	ret.y /= 2;
	return ret;
}

template<typename num_t>
void reorderClockWise(std::vector<cv::Point_<num_t> >& points) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
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
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
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

template<typename num_t, size_t s>
std::array<cv::Point3_<num_t>, s> operator+(const std::array<cv::Point_<num_t>, s>& base, num_t depth) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	std::array<cv::Point3_<num_t>, s> ret;
	for(size_t i = 0; i < s; i++) {
		ret[i] = std::move(cv::Point3_<num_t>(base[i].x, base[i].y, depth));
	}
	return ret;
}
template<typename num_t, size_t s>
std::array<cv::Point3_<num_t>, s> operator+(const std::array<cv::Point3_<num_t>, s>& base, num_t depth) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	std::array<cv::Point3_<num_t>, s> ret;
	for(size_t i = 0; i < s; i++) {
		ret[i] = base[i];
		ret[i].z += depth;
	}
}
template<typename num_t, size_t s>
void operator+=(std::array<cv::Point3_<num_t>, s>& base, num_t depth) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	for(size_t i = 0; i < s; i++) {
		base[i].z += depth;
	}
}
template<typename num_t>
std::vector<cv::Point3_<num_t> > operator+(const std::vector<cv::Point_<num_t> >& base, num_t depth) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	std::vector<cv::Point3_<num_t> > ret(base.size());
	for(size_t i = 0; i < base.size(); i++) {
		ret.emplace_back(base[i].x, base[i].y, depth);
	}
	return ret;
}
template<typename num_t>
std::vector<cv::Point3_<num_t> > operator+(const std::vector<cv::Point3_<num_t> >& base, num_t depth) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	std::vector<cv::Point3_<num_t> > ret(base.size());
	for(size_t i = 0; i < base.size(); i++) {
		ret.emplace_back(base[i].x, base[i].y, base[i].z + depth);
	}
	return ret;
}
template<typename num_t>
void operator+=(std::vector<cv::Point3_<num_t>>& base, num_t depth) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	for(size_t i = 0; i < base.size(); i++) {
		base[i].depth += depth;
	}
}