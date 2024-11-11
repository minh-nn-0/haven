#pragma once

#include <string>
#include <mmath/core.hpp>

namespace haven
{
namespace character
{
	struct dialogue
	{
		void update(float dtr)
		{
			if (is_active()) _timer -= dtr; 
		};
		void new_content(const std::string& content)
		{
			_content = content;
			_timer = content.length() * _mspc;
		};
		std::string _content;
		int _timer {0};
		float _mspc {60*17}; //milliseconds per character
							 // remember to time milliseconds per frame
							 // (default to 60 * 17 milliseconds per frame)
		bool is_active() {return _timer >= 0;};
	};
};
};
