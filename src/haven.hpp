#include <Beaver/core.hpp>
#include <print>
#include "haven/world.hpp"
namespace haven
{
	struct player_control
	{
		int _input;
		bool _repeat; // required pressed or just_pressed
		character::action::info _action;
	};

	struct game
	{
		game();
		void run();
		
		unsigned				_player_cid;
		std::array<player_control, 8> _player_control;
		void update_available_action();
		
		haven::world			_world;
		beaver::camera2D		_cam;
		
		beaver::event_system::queued<haven::EVENT>	_events;
		
		tiled::tileset			_tileset;
		// windowing, rendering, input, assets (texture, audio, font)
		beaver::sdlgame			_sdl;
	};

	//auto available_player_control(int player_cid, 
	//		const control_map& player_control_map, 
	//		const haven::world& world);
};

