#pragma once

#include <Beaver/core.hpp>
#include <set>

namespace haven
{
	//enum class character_type
	//{
	//	player,
	//	npc
	//};
	
	
	//enum class DIRECTION {UP, DOWN, LEFT, RIGHT};

	//template<DIRECTION Direction>
	//struct move : __action<beaver::sprite>
	//{
	//	virtual bool pred()
	//	{
	//	};

	//	virtual action_result execute(beaver::sprite& actor, float dt)
	//	{
	//		float dt_ratio = dt/17.0;
	//		using enum vex::DIRECTION;

	//		if constexpr (Direction == UP)
	//			actor._rect._pos.y -= _vspeed * dt_ratio;
	//		else if constexpr (Direction == DOWN)      
	//			actor._rect._pos.y += _vspeed * dt_ratio;
	//		else if constexpr (Direction == LEFT)      
	//			actor._rect._pos.x -= _hspeed * dt_ratio;
	//		else if constexpr (Direction == RIGHT)     
	//			actor._rect._pos.x += _hspeed * dt_ratio;
	//	};

	//	float	_hspeed {0.5};
	//	float	_vspeed {0.5};
	//};

	//struct __goal
	//{
	//	int _priority;
	//	virtual bool satisfied();
	//};

	//struct character
	//{
	//	//struct action
	//	//{
	//	//	int 					_priority;
	//	//	std::function<bool()>	_start_cond;
	//	//	std::function<void(character&, int dt)>	_action;

	//	//	//TODO: add effects (like world state after action)

	//	//	auto operator <=> (const action& r1) const
	//	//	{
	//	//		//return _priority <= r1._priority ? std::strong_ordering::equal
	//	//		//	: _priority < r1._priority ? std::strong_ordering::less
	//	//		//								: std::strong_ordering::greater;
	//	//		auto cond = _start_cond() <=> r1._start_cond();
	//	//		if (cond != 0) return cond;
	//	//		return _priority <=> r1._priority;
	//	//	};
	//	//	bool operator == (const action& r1) const
	//	//	{
	//	//		return _start_cond() == r1._start_cond() && _priority == r1._priority;
	//	//	};

	//	//};
	//	//struct status
	//	//{
	//	//	const action* 	_current_action;
	//	//	int				_mood {0};
	//	//	bool			_free{true};
	//	//};
	//	
	//	// PROPERTIES
	//	
	//	std::string				_name;
	//	beaver::sprite			_spr;
	//	
	//	beaver::fsm<CHARACTER_STATE> _fsm;
	//	std::bitset<16>			_flags;
	//	
	//	beaver::event_system::immediate<vex::EVENT>	_event_handler;

	//	//std::set<routines>		_routines;
	//	void update(const int dt) 
	//	{
	//		//auto* cur_action = _status._current_action;
	//		//if (_free)
	//		//{
	//		//	auto available_routines = _routines | std::views::filter([](auto&& r){return r._start_cond();});
	//		//	if (!available_routines.empty()){cur_action = &available_routines.back()._action;};
	//		//};
	//		//if (cur_action != nullptr) (*cur_action)(*this, dt);
	//	};
	//};


	//template<unsigned... Flags>
	//constexpr bool check_flags(const character& c)
	//{
	//	return (c._flags.test(Flags),...);
	//};

	//namespace character_actions
	//{
	//	constexpr int CHARACTER_SPEED = 2; //pixel per frame
	//	constexpr int CHARACTER_CLIMB_SPEED = 1; //pixel per frame

	//	// Character can only move sideway or climb ladder, no jump, no flying etc...
	//	void move_h(vex::character&, int dt);
	//	void move_v(vex::character&, int dt); //aka climb ladder
	//	// If want to move to arbitrary location, character need to be aware of the surroundings 
	//	// (aka the world, or maps maybe)
	//	// or have to move the character from perspective of the game. like a puppet
	//	//
	//	//
	//	void move(vex::character&, int dt, const mmath::ivec2& dest);
	//};
	
namespace character
{
	enum class STATE
	{
		IDLE, SIT, MOVE, JUMP, LEAN, TALK,
	};

	namespace FLAGS
	{
		constexpr char 	ONGROUND 		= 0,
				  		FACING_LEFT 	= 1,
						IN_LADDER 		= 2,
						FREE 			= 3;
	};

	// can't use name as string, location is string already ;(
	
	struct info
	{
		std::string _name;
		// TODO personality, work;
	};

	// index to tiled group index
	// "Outdoor" is 0, "Hall" is 1 etc.....
	// using location = unsigned;
	
	// updated, use string for location ;)
	
	using location = std::string;
	using flags = std::bitset<16>;

	using fsm = beaver::fsm<STATE>;
	


	struct knowledge
	{
		std::vector<int> _self;
		std::vector<int> _characters;
		std::vector<int> _world;
	};
	

	namespace action
	{
		enum class result {SUCCESS, FAILURE, RUNNING, NOTINIT};
		inline std::string print_result(result a)
		{
			switch (a)
			{
				case (result::SUCCESS): return "SUCCESS";
				case (result::RUNNING): return "RUNNING";
				case (result::FAILURE): return "FAILURE";
				case (result::NOTINIT): return "NOTINIT";
			};
		};
		enum class type
		{
			MOVE, MOVE_TO, JUMP,
			TALK, OPEN_DOOR, CLIMB_LADDER,
		};
		inline std::string print_type(type t)
		{
			switch (t)
			{
				case (type::MOVE): return "MOVE";
				case (type::MOVE_TO): return "MOVE TO";
				case (type::TALK): return "TALK";
				case (type::JUMP): return "JUMP";
				case (type::OPEN_DOOR): return "OPEN_DOOR";
				case (type::CLIMB_LADDER): return "CLIMB_LADDER";
			};
		};

		using param = std::variant<std::monostate, int, float, mmath::fvec2, std::string>;
		using pred = std::function<bool(int)>;
		using func = std::function<result(int, const param&, float)>;

		using info = std::tuple<type, param>;
		using record = std::tuple<info, result>;

		struct execution
		{
			pred _pred;
			func _func;
		};

		using execution_map = std::unordered_map<type, execution>;
	};

	struct transform
	{
		mmath::fvec2 _velocity;
	};
	
	// same spirit as behavior tree
	//struct task
	//{
	//	std::vector<action::info> _actions;
	//	int _current_action {0};
	//	action::result  _last_result;
	//	
	//	action::info& current_action_info() {return _actions.at(_current_action);};

	//	template<typename Fn>
	//	action::result execute(int cid, const Fn& fn, float dtr)
	//	{
	//		_last_result = fn(cid, std::get<action::param>(current_action_info()), dtr);
	//		if (_last_result == action::result::SUCCESS)
	//		{
	//			if (_current_action == _actions.size() - 1) return action::result::SUCCESS;
	//			else
	//			{
	//				_last_result = action::result::RUNNING;
	//				_current_action++;
	//			};
	//		};
	//		
	//		return _last_result;
	//	};

	//	void reset()
	//	{
	//		_current_action = 0;
	//	};
	//};

	
	//struct action
	//{
	//	action_rs execute()
	//	{
	//		if (!_pred()) _last_rs =  action_rs::FAILURE;
	//		else _last_rs = _action();

	//		return _last_rs;
	//	};
	//	std::function<bool()> _pred = [](){return true;};
	//	std::function<action_rs()> _action;
	//	int _cost;
	//	action_rs _last_rs;
	//};



};
};
