#pragma once

#include <array>
#include <type_traits>


namespace vs2 {

enum class BGR {
	BLUE = 0,
	GREEN = 1,
	RED = 2
};

template<typename t = uint16_t>
inline t operator~(BGR v) {
	static_assert(std::is_arithmetic<t>::value, "Template paramter (t) must be arithmetic type.");
	return static_cast<t>(v);
}

inline static std::array<std::array<uint8_t, 2>, 3> weights_map{
	std::array<uint8_t, 2>{~BGR::GREEN, ~BGR::RED},		// blue
	std::array<uint8_t, 2>{~BGR::BLUE, ~BGR::RED},		// green
	std::array<uint8_t, 2>{~BGR::BLUE, ~BGR::GREEN},	// red
};

} // namespace vs2