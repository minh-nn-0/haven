#pragma once

#include "character.hpp"
#include "events.hpp"
#include "dialogue.hpp"
namespace haven
{

	struct location
	{
		struct door
		{
			std::string _destination;
			mmath::frect _rect;
		};

		tiled::tilemap _foreground, _background, _ground;
		std::vector<door> _doors;
	};
	
	struct world
	{
		world();
		
		// tile information
		int _tilesize, _numx, _numy;
	
		
		// Timing
		beaver::normal_clock<17>		_clock;

		std::unordered_map<std::string, location>	_locations;

		// Terrain info and collision

		enum class TERRAIN
		{
			GRASS,
			WATER,
			DIRT,
			WOOD,
			BRICK,
			LADDER
		};

		struct terrain_info
		{
			std::set<int> _tiles;
			float _friction;
		};
		const std::unordered_map<TERRAIN, terrain_info> _terrains
		{
			{TERRAIN::GRASS, {{65}, 0.2}},
			{TERRAIN::WATER, {{}}},
			{TERRAIN::DIRT, {{2502}, 0.1}},
			{TERRAIN::WOOD, {{7028, 7033}, 0.1}},
			{TERRAIN::BRICK, {{6589}, 0.1}},
			{TERRAIN::LADDER, {{5916, 5918}, 0}},
		};

		// up down left right
		std::array<long, 4> get_surrounding_tiles(int);
		long get_current_tile(int);
		

		TERRAIN terrain_type(int tile)
		{
			return std::ranges::find_if(_terrains, [&](auto&& terrain)
					{
						return terrain.second._tiles.contains(tile);
					})->first;
		};
		template<TERRAIN... Ts>
		constexpr bool tile_is(long id) 
		{
			auto [_, tile] = tiled::get_flipflags(id);
			return (_terrains.at(Ts)._tiles.contains(tile) && ...); 
		};

		template<TERRAIN... Ts>
		constexpr bool tile_is_one_of(long id)
		{
			auto [_, tile] = tiled::get_flipflags(id);
			return (_terrains.at(Ts)._tiles.contains(tile) || ...);
		};
		
		location::door* character_near_door(int);

		void apply_friction(int, float);

		const float _GRAVITY {0.5};
		void apply_gravity(int, float);

		//constexpr bool can_stand(int cid);
		//constexpr bool near_door(int cid);
		//constexpr bool near_ladder(int cid);

		// Characters
		beaver::entity::manager<character::info, 
								character::location,
								character::dialogue,
								character::action::record,
								beaver::sprite, 
								character::transform,
								character::flags, 
								character::fsm>	_characters;

		unsigned add_character(const std::string& name, const mmath::fvec2& pos,
				const std::string& location);
		
		void process_flags(int, float);
		void process_states(int);
		void process_transformation(int, float);

		// Actions
		struct actions
		{
			std::size_t _cid;
			character::action::record _record;
		};
		std::vector<actions> _ongoing_actions;
		
		character::action::execution_map _actions;
		
		bool character_can_perform(int, character::action::type) const;
		
		character::action::result character_perform_immediate(int, const character::action::info&, float);

		void character_perform_queue(int, const character::action::info&);
		
		void resolve_character_actions(float);
		//// Tasks
		//std::unordered_map<std::string, character::task> _tasks;
			// Primitive
		//character::action character_idle(std::size_t c);
		//character::action character_move(std::size_t c, const mmath::fvec2& movement, float dtr);
		//character::action character_talk(std::size_t c, const std::string& content);
		//character::action character_sit(std::size_t c);
		//character::action character_lean(std::size_t c);
		
			// Compound
		//character::action character_move_to(std::size_t c, const mmath::fvec2& destination, float dtr);
		

		beaver::event_system::immediate<haven::EVENT> _event_handler;
		beaver::event_system::queued<haven::EVENT>	_event_queue;



		void update(float, const beaver::controller&);
	};


	namespace world_location
	{
		constexpr unsigned 	OUTDOOR = 0,
				  			HALL	= 1,
							FOREST 	= 2,
							HOME	= 3,
							HOUSE1	= 4,
							HOUSE2	= 5,
							HOUSE3	= 6,
							HOUSE4	= 7;
	};
	

};

