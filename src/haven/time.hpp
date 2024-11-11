#pragma once

#include <Beaver/time.hpp>

namespace vex
{
	inline std::array<unsigned, 3> get_time(const beaver::normal_clock cl) {return {cl._h, cl._m, cl._s};};

};
