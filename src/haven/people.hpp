#include <Beaver/core.hpp>
#include "time.hpp"

namespace vex
{
	template<typename T>
	concept people = requires (T x) { x._name; x._routines; };
}




