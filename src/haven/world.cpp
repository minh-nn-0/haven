#include "world.hpp"
#include <fstream>

// UP DOWN LEFT RIGHT and CURRENT
std::array<long, 4> vex::world::get_surrounding_tiles(int cid)
{
	std::array<long, 4> rs;
	
	auto [spr, location_name] = _characters.get_components<beaver::sprite, character::location>(cid);

	const tiled::tilelayer& ground = std::get<tiled::tilelayer>(_locations.at(*location_name)._ground._layersdata[0]);
	mmath::ivec2 spr_tile = tiled::id_at(spr->_rect._pos, _tilesize);

	bool out_of_bound {false};
	
	if (spr_tile.x < 0 || spr_tile.x >= ground._numx
		|| spr_tile.y < 0 || spr_tile.y >= ground._numy) out_of_bound = true;
	
	if (!out_of_bound)
	{
		int spr_tileid = spr_tile.x + spr_tile.y * ground._numx;
		std::array<int, 4> surrounding_tileids {spr_tileid - ground._numx,
												spr_tileid + ground._numx,
												spr_tileid - 1,
												spr_tileid + 1};
		
		for (int i = 0; i != 4; i++) 
			rs[i] = ground._data[surrounding_tileids[i]];

	}
	else rs.fill(-1);

	return rs;
};

vex::location::door* vex::world::character_near_door(int cid)
{
	auto [spr, char_location] = _characters.get_components<beaver::sprite, character::location>(cid);

	location& current_location = _locations.at(*char_location);

	if (auto door_found = std::ranges::find_if(current_location._doors, 
				[=](auto&& door) 
				{ 
					return mmath::is_intersecting(spr->_rect.center(), door._rect);
				});
		 door_found != std::end(current_location._doors))
		return &*door_found;
	else return nullptr;
};

void vex::world::apply_gravity(int cid, float dtr)
{
	auto [spr, flags] = _characters.get_components<beaver::sprite, character::flags>(cid);
	if (! utils::check_bits<character::FLAGS::ONGROUND>(*flags))
		spr->_rect._pos.y += _GRAVITY * dtr;
};

void vex::world::process_states(int cid)
{
	auto* fsm = _characters.get_component<character::fsm>(cid);

	if (fsm->_cur != character::STATE::IDLE)
	{
		// If not moving
		if (_characters.get_component<character::transform>(cid)->_velocity == mmath::fvec2{0,0})
			fsm->set(character::STATE::IDLE);
	};
};

void vex::world::process_flags(int cid, float dtr)
{
	auto [flags, transform] = _characters.get_components<character::flags, character::transform>(cid);

	auto surrounding_tiles = get_surrounding_tiles(cid);
	if (transform->_velocity.y >= 0 
			&& tile_is_one_of<TERRAIN::DIRT,
								TERRAIN::BRICK,
								TERRAIN::GRASS,
					TERRAIN::WOOD>(surrounding_tiles[1]))
		flags->set(character::FLAGS::ONGROUND);
	else flags->reset(character::FLAGS::ONGROUND);
};

void vex::world::apply_friction(int cid, float dtr)
{
	auto [flags, transform] = _characters.get_components<character::flags, character::transform>(cid);
	
	constexpr float AIR_FRICTION = 0.1;

	float friction = AIR_FRICTION; 

	if (flags->test(character::FLAGS::ONGROUND))
	{
		TERRAIN under_tile = terrain_type(get_surrounding_tiles(cid)[1]);
		friction = _terrains.at(under_tile)._friction;
	}

	if (std::abs(transform->_velocity.x) >= 0.3)
	{
		int direction = transform->_velocity.x > 0 ? -1 : 1;
		transform->_velocity.x += friction * direction * dtr;
	}
	else transform->_velocity.x = 0;
};

void vex::world::process_transformation(int cid, float dtr)
{
	auto [spr, flags, fsm, transform] = _characters.get_components<beaver::sprite,
																character::flags,
																character::fsm,
																character::transform>(cid);
	apply_gravity(cid, dtr);

	apply_friction(cid, dtr);
	

	if (utils::check_bits<character::FLAGS::ONGROUND>(*flags))
		transform->_velocity.y = 0;
	

	if (transform->_velocity.y < 0)
	{
		transform->_velocity.y += _GRAVITY * dtr; 
	};

	if (transform->_velocity.x < 0) 
		flags->set(character::FLAGS::FACING_LEFT);
	else if (transform->_velocity.x > 0)
		flags->reset(character::FLAGS::FACING_LEFT);
	
	spr->_rect._pos += transform->_velocity * dtr;


};

void vex::world::resolve_character_actions(float dtr)
{
	for (auto& [cid, record]: _ongoing_actions)
	{
		auto& [action_info, action_rs] = record;
		auto* character_action_record = _characters.get_component<character::action::record>(cid);
		action_rs = std::get<character::action::result>(*character_action_record) = character_perform_immediate(cid, action_info, dtr);
		std::println("cid {}, action type {}", cid, character::action::print_type(std::get<character::action::type>(action_info)));
	};

	auto [rf, rl] = std::ranges::remove_if(_ongoing_actions, [](auto&& info)
											{
												auto& [_, action_info] = info;
												auto& result = std::get<character::action::result>(action_info);
												return result == character::action::result::SUCCESS || result == character::action::result::FAILURE;
											});
	_ongoing_actions.erase(rf, rl);

};


void vex::world::update(float dtr, const beaver::controller& ctl)
{
	_clock.tick(dtr);

	dtr = _clock._spms/1000.f;
	
	resolve_character_actions(dtr);
	for (std::size_t cid = 0; cid != _characters.size(); cid++)
	{
		auto [spr, location, flags, fsm, transform] = _characters.get_components<beaver::sprite,
																		character::location,
																		character::flags, 
																		character::fsm, 
																		character::transform>(cid);
		process_states(cid);	
		process_flags(cid, dtr);
		process_transformation(cid, dtr);

		for (auto& door : _locations.at(*location)._doors)
		{
			if (mmath::is_intersecting(spr->_rect.center(), door._rect))
			{
				std::println("intersect withd door");
				if (ctl.just_pressed(beaver::BTNX))
				{
					*location = door._destination;
				};
			};
		};

		//if (!flags->test(character::FLAGS::FREE))
		//{
		//	//auto* task = _characters.get_component<std::function<character::action::result()>>(i);
		//	//if (auto task_result = (*task)(); 
		//	//		task_result == character::action::result::SUCCESS
		//	//		|| task_result == character::action::result::FAILURE)
		//	//	flags->set(character::FLAGS::FREE);
		//	
		//	//auto& [task_current_action_type, task_current_action_param] = _characters.get_component<character::task>(i)->current_action_info();
		//	//
		//	//if (_action_preds.at(task_current_action_type)(i))
		//	//	_action_functions.at(task_current_action_type)(i, task_current_action_param, dtr);
		//	
		//	
		//	//if (action->execute() == character::action_rs::FAILURE)
		//	//	flags->set(character::FLAGS::FREE);

		//	



		//	//if (action->_pred())
		//	//{
		//	//	action->_last_rs = action->_action();
		//	//	if (action->_last_rs == character::action_rs::SUCCESS) flags->set(character::FLAGS::FREE); 
		//	//}
		//	//else 
		//	//{
		//	//	// pred failed
		//	//	action->_last_rs = character::action_rs::FAILURE;
		//	//	flags->set(character::FLAGS::FREE);
		//	//};
		//};

		// dialogues
		_characters.get_component<character::dialogue>(cid)->update(dtr);
		
		// update animation
		if (!flags->test(character::FLAGS::IN_LADDER))
			spr->_tileanimation.update(dtr);
	};


		
};

unsigned vex::world::add_character(const std::string& name, const mmath::fvec2& pos, 
		const std::string& location)
{
	unsigned new_char = _characters.add_entity();
	auto [cinfo, cspr, clocation] = _characters.get_components<character::info, beaver::sprite, character::location>(new_char); 

	cspr->_rect = {pos, 8,8};
	cinfo->_name = name;
	*clocation = location;
	
	return new_char;
};


//vex::character::action vex::world::character_idle(std::size_t c)
//{
//	return {._pred = [](){return true;},
//			._action = [&, c]()
//			{ 
//				_characters.get_component<character::fsm>(c)->set(character::STATE::IDLE);
//				return vex::character::action_rs::SUCCESS;
//			}};
//
//};
//
//vex::character::action vex::world::character_move(std::size_t c, const mmath::fvec2& movement, float dtr)
//{
//	return {._action = [&, c, movement, dtr]()
//			{
//				auto [spr, fsm, flag] = _characters.get_components<beaver::sprite, character::fsm, character::flags>(c);
//				if (!flag->test(character::FLAGS::IN_LADDER))
//				{
//					fsm->set(character::STATE::MOVE);
//					
//					if (movement.x < 0) flag->set(vex::character::FLAGS::FACING_LEFT);
//					else flag->reset(vex::character::FLAGS::FACING_LEFT);
//					
//					spr->_rect._pos += movement * dtr;
//					return character::action_rs::SUCCESS;
//				}
//				else 
//				{
//					spr->_tileanimation.update(dtr);
//					spr->_rect._pos.y += movement.y * dtr;
//
//					return character::action_rs::SUCCESS;
//				};
//			}};
//};
//
//vex::character::action vex::world::character_talk(std::size_t c, const std::string& content)
//{
//	return {._action = [&,c]()
//			{
//				_characters.get_component<character::dialogue>(c)->new_content(content);
//				return character::action_rs::SUCCESS;
//			}};
//};
//
//vex::character::action vex::world::character_move_to(std::size_t c, const mmath::fvec2& destination, float dtr)
//{
//	return {._action = [&, c, destination, dtr]
//			{
//				auto* spr = _characters.get_component<beaver::sprite>(c);
//				if (std::abs(destination.x - spr->_rect._pos.x) < 0.5)
//				{
//					character_idle(c).execute();
//					_characters.get_component<character::flags>(c)->set(character::FLAGS::FREE);
//					return character::action_rs::SUCCESS;
//				};
//				int direction = destination.x - spr->_rect._pos.x > 0 ? 1 : -1;
//				auto move = character_move(c, mmath::fvec2{0.6f * direction, 0}, dtr);
//				
//				if (move.execute() == character::action_rs::FAILURE) return character::action_rs::FAILURE;
//				
//				return character::action_rs::RUNNING;
//			}};
//};



std::unordered_map<std::string, vex::location> load_map(tiled::tilemap& tmj)
{
	
	std::unordered_map<std::string, vex::location> rs;
	tiled::tilemap base;
	base._tilesize = tmj._tilesize;
	base._numx = tmj._numx;
	base._numy = tmj._numy;
	base._bgcolor = tmj._bgcolor;
	base._tilesets = tmj._tilesets;

	for (auto& layer: tmj._layersdata)
	{
		if (tiled::grouplayer* group = std::get_if<tiled::grouplayer>(&layer))
		{
			vex::location current_location {._foreground = base,
										._background = base,
										._ground = base};

			current_location._foreground._layersdata.emplace_back(group->get_layer_by_name("Foreground"));
			current_location._background._layersdata.emplace_back(group->get_layer_by_name("Background"));
			current_location._ground._layersdata.emplace_back(group->get_layer_by_name("ground"));

			
			if (group->have_layer("Doors"))
			{
				std::println("DOOOR");
				for (auto& doors: std::get<tiled::objectlayer>(group->get_layer_by_name("Doors"))._objects)
				{
					if (const mmath::frect* door_rect = std::get_if<mmath::frect>(&doors._object))
					{
						std::string destination = doors._properties.at("Destination");
						std::println("door {}, pos {} {}, size {} {}", destination, door_rect->_pos.x,
																					door_rect->_pos.y,
																					door_rect->_size.x,
																					door_rect->_size.y);
						current_location._doors.emplace_back(destination, *door_rect);
					};
				};
			};

			rs.emplace(group->_name, current_location);
		};
	};

	return rs;
};




vex::world::world()
{
	// MAP
	
	tiled::tilemap tmj {std::string(DATA_PATH) + "map1.tmj"};
	
	_tilesize = tmj._tilesize;
	_numx =	tmj._numx;
	_numy = tmj._numy;

	_locations = load_map(tmj);

	// CHARACTERS
	std::ifstream f;
	f.open(std::string(DATA_PATH) + "characters");
	
	int start_xpos {100};
	std::string character;
	while (std::getline(f, character))
	{
		std::stringstream ss {character};
		std::string charname; int base_tileid;
		ss >> charname; ss >> base_tileid;
		
		unsigned newchar = add_character(charname, mmath::ivec2{start_xpos, 16} * 8, "Outdoor");
		start_xpos++;
		
		_characters.get_component<beaver::sprite>(newchar)->_tileanimation.new_frames({{400, base_tileid}, {400, base_tileid + 1}});
		
		auto* charfsm = _characters.get_component<character::fsm>(newchar);
		
		using enum character::STATE;
		charfsm->_cur = IDLE;
		
		auto set_animation_idle = [&, base_tileid, newchar]
		{
			_characters.get_component<beaver::sprite>(newchar)->_tileanimation
							.new_frames({{400, base_tileid + 1}, {400, base_tileid}});
		};

		auto set_animation_move = [&, base_tileid, newchar]
		{
			_characters.get_component<beaver::sprite>(newchar)->_tileanimation
							.new_frames({{400, base_tileid + 2}, {400, base_tileid}});
		};
		auto set_animation_jump = [&, base_tileid, newchar]
		{
			_characters.get_component<beaver::sprite>(newchar)->_tileanimation
				.new_frames({{400, base_tileid + 3}});
		};
		charfsm->_transition_table[IDLE][MOVE] = set_animation_move;
		charfsm->_transition_table[IDLE][JUMP] = set_animation_jump;

		charfsm->_transition_table[MOVE][IDLE] = set_animation_idle;
		charfsm->_transition_table[MOVE][JUMP] = set_animation_jump;
		

		charfsm->_transition_table[JUMP][IDLE] = set_animation_idle;
		charfsm->_transition_table[JUMP][MOVE] = set_animation_move;
		
		
		_characters.get_component<character::flags>(newchar)->set(character::FLAGS::FREE);

	};



//	_actions["move"] = {._pred = [&]()
//							{
//								return true;
//							},
//						._action = [&]()
//							{
//								return character::action_rs::SUCCESS;
//							}};
//
	_actions[character::action::type::MOVE] =
		{._pred = [&](int cid){return _characters.get_component<character::flags>(cid)->test(character::FLAGS::FREE);},
		 ._func = [&](int cid, const character::action::param& aparam, float dtr)
			{
				if (const float* movement = std::get_if<float>(&aparam))
				{
					auto [flags, fsm, transform] = _characters.get_components<character::flags,
																			character::fsm,
																			character::transform>(cid);

					fsm->set(character::STATE::MOVE);
					transform->_velocity.x = *movement;
					
					return character::action::result::SUCCESS;
				}
				else throw std::invalid_argument("expected fvec2 for movement");
			}
		};

	_actions[character::action::type::MOVE_TO] =
		{._pred = [](int cid){return true;},
		 ._func = [&](int cid, const character::action::param& aparam, float dtr)
			{
				if (const mmath::fvec2* destination = std::get_if<mmath::fvec2>(&aparam))
				{
					auto [spr, flags] = _characters.get_components<beaver::sprite, character::flags>(cid);

					float gap = destination->x - spr->_rect._pos.x;
					
					if (std::abs(gap) <= 0.5) 
					{
						return character::action::result::SUCCESS;
					}
					else
					{
						int direction = gap > 0 ? 1 : -1;
						character_perform_immediate(cid, {character::action::type::MOVE, 0.7f * direction}, dtr);
						return character::action::result::RUNNING;
					};
				}
				else throw std::invalid_argument("expected float for destination");
			}
		};
	_actions[character::action::type::TALK] = 
		{._pred = [](int cid){return true;},
		 ._func = [&](int cid, const character::action::param& aparam, float dtr)
			{
				if (const std::string* content = std::get_if<std::string>(&aparam))
				{
					_characters.get_component<character::dialogue>(cid)->new_content(*content);
					return character::action::result::SUCCESS;
				}
				else
				{
					throw std::invalid_argument("expected string for dialogue content");
				};
			}
		};
	_actions[character::action::type::JUMP] =
		{._pred = [&](int cid)
			{
				return utils::check_bits<character::FLAGS::ONGROUND>(*_characters.get_component<character::flags>(cid));
			},
		._func = [&](int cid, const character::action::param& aparam, float dtr)
			{
				if (const float* jumpforce = std::get_if<float>(&aparam))
				{
					auto [flags, fsm, transform] = _characters.get_components<character::flags,
																			character::fsm,
																			character::transform>(cid);
					if (utils::check_bits<character::FLAGS::ONGROUND>(*flags))
					{
						transform->_velocity.y -= *jumpforce;

						fsm->set(character::STATE::JUMP);
						flags->reset(character::FLAGS::ONGROUND);
						return character::action::result::SUCCESS;
					}
					else return character::action::result::FAILURE;
				}
				else
				{
					throw std::invalid_argument("expected float for jump");
				};
			}
		};

	_actions[character::action::type::OPEN_DOOR] =
		{._pred = [&](int cid){return character_near_door(cid) != nullptr;},
		 ._func = [&](int cid, const character::action::param& aparam, float dtr)
		 	{
				auto* door_to_open = character_near_door(cid);

				_characters.get_component<character::location>(cid)->assign(door_to_open->_destination);
				return character::action::result::SUCCESS;
		 	}
		};

};

bool vex::world::character_can_perform(int cid, character::action::type atype) const
{
	return _actions.at(atype)._pred(cid);
};
vex::character::action::result vex::world::character_perform_immediate(int cid, const character::action::info& ainfo, float dtr)
{
	auto* flags = _characters.get_component<character::flags>(cid);
	auto& [type, param] = ainfo;
	if (!_actions.at(type)._pred(cid))
		return character::action::result::FAILURE;
	else
	{
		flags->set(character::FLAGS::FREE);
		return _actions.at(type)._func(cid, param, dtr);
	};
};

void vex::world::character_perform_queue(int cid, const character::action::info& ainfo)
{
	_ongoing_actions.emplace_back(cid, 
			character::action::record{ainfo, character::action::result::NOTINIT});
};


