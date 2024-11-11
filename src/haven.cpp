#include "haven.hpp"

void load_assets(beaver::sdlgame::assets_manager& assets, SDL_Renderer* rdr)
{
	// Images
	assets.add<sdl::texture>("tileset",
				{(std::string(TILESETS_PATH) + "havened_ChromaNoir.png").c_str(), rdr});

	std::println("{}",assets.get_map<sdl::texture>().at("tileset")->_name);

	assets.add<sdl::texture>("ui_keyboard",
				{(std::string(IMAGES_PATH) + "UI/icons-keyboard-16x16-1bit-ansdor.png").c_str(), rdr});

	// Audios
	
	assets.add<sdl::soundchunk>("coin single", 
			{(std::string(AUDIOS_PATH) + "coin_single.wav").c_str()});
	assets.add<sdl::soundchunk>("coin single2", 
			{(std::string(AUDIOS_PATH) + "coin2.wav").c_str()});

	assets.add<sdl::music>("thenightislate", {(std::string(AUDIOS_PATH) + "thenightislate.mp3").c_str()});
	// Fonts 

	assets.add<sdl::font>("terminess", {(std::string(FONTS_PATH) + "TerminessNerdFontMono-Bold.ttf").c_str(), 16});
	
};

//std::vector<haven::character> make_npcs()
//{
//	using enum haven::CHARACTER_STATE;
//	
//	std::vector<haven::character> rs;
//	
//	auto& s = rs.emplace_back(haven::character{._name = "bob"}); 
//	s._fsm._cur = IDLE;
//	s._fsm._transition_table[IDLE] = 
//	{
//		{MOVE, 
//			[&]()
//			{
//				s._spr._tileanimation = {};
//			}},
//		{CLIMB, 
//			[&]()
//			{
//			}},
//		{}
//	};
//
//	return rs;
//};

//std::vector<tiled::tilemap> load_maps()
//{
//	std::vector<tiled::tilemap> rs;
//	rs.emplace_back(std::string(DATA_PATH) + "map1.tmj");
//
//	return rs;
//};

void gamemenu()
{
};

//auto haven::available_player_control(int player_cid,
//											const haven::control_map& player_control_map,
//											const haven::world& world)
//{
//	return player_control_map 
//			| std::views::filter(
//				[&](auto&& pcontrol)
//				{
//					character::action::type atype = std::get<character::action::type>(pcontrol._action);
//					return world.character_can_perform(player_cid,atype);
//				})
//			| std::views::transform(
//				[](auto&& pcontrol)
//				{return std::get<character::action::type>(pcontrol._action);});
//													
//};

void haven::game::update_available_action()
{
	using enum character::action::type;
	std::set<character::action::type> non_interaction_actions = {MOVE, MOVE_TO, JUMP};
	auto first_available_interactions = std::ranges::find_if(_world._actions,
										[&](auto&& action) 
										{
											return !non_interaction_actions.contains(action.first)
												&& _world.character_can_perform(_player_cid, action.first);
										});
		
};


float dialogue_scale {1};
bool havened_update(haven::game& haven, float dtr)
{
	using enum haven::character::action::type;
	auto* pspr = haven._world._characters.get_component<beaver::sprite>(haven._player_cid);
	const beaver::controller& ctl = haven._sdl._ctl;
	//if (haven._world._characters.get_component<haven::character::flags>(haven._player_cid)->test(haven::character::FLAGS::FREE))
	//{
	//	if (ctl.pressed(beaver::BTNL)) 
	//		haven._world.character_perform_immediate(haven._player_cid, {MOVE, -0.7f}, dtr);
	//	if (ctl.pressed(beaver::BTNR)) 
	//		haven._world.character_perform_immediate(haven._player_cid, {MOVE, 0.7f}, dtr);
	//	if (ctl.pressed(beaver::BTNU)) 
	//		haven._world.character_perform_immediate(haven._player_cid, {JUMP, 4.f}, dtr);
	//	//if (ctl.pressed(beaver::BTND)) 
	//	//	haven._world.character_perform_immediate(haven._player_cid, {MOVE, mmath::fvec2{0.6, 0}}, dtr);
	//};

	for (auto& control: haven::available_player_control(haven._player_cid,
													haven._player_control, haven._world))
	{
		bool input_received = control._repeat?
			ctl.pressed(control._input)
			: ctl.just_pressed(control._input);

		if (input_received) 
			haven._world.character_perform_immediate(haven._player_cid, control._action, dtr);
	};
	std::println("{} {}", pspr->_rect._pos.x, pspr->_rect._pos.y);

	haven._world.update(dtr, haven._sdl._ctl);
	
	//if (haven._sdl._ctl.just_pressed(beaver::BTNZ)) 
	//{
	//	std::println("==================================");
	//	Mix_PlayChannel(-1, *haven._sdl._assets.get<sdl::soundchunk>("coin single2"), 0);
	//};

	//haven._world._clock.tick(dtr);
	//auto [h,m,s] = haven._world._clock.get_time();
	//std::println("time of day {}, {}, {}", h,m,s);
	
	// World update
	
	
	// update camera
	SDL_Renderer* rdr = haven._sdl._sdl._renderer;
	int ww, wh;
	SDL_GetRendererOutputSize(rdr, &ww, &wh);
	haven._cam._view._size.x = ww;
	haven._cam._view._size.y = wh;

	haven._cam.target(pspr->_rect.center(), dtr);


#ifndef NDEBUG
	
	std::string* plocation = haven._world._characters.get_component<haven::character::location>(haven._player_cid);
	haven::character::dialogue* pdialogue = haven._world._characters.get_component<haven::character::dialogue>(haven._player_cid);
	
	static int spms{1000};
	haven._world._clock._spms = spms;
	
	ImGui::Begin("dialogue");
	
	//if (ImGui::Button("outdoor")) *plocation = "Outdoor";
	//if (ImGui::Button("Indoor1")) *plocation = "Indoor1";
	//if (ImGui::Button("Indoor2")) *plocation = "Indoor2";

	if (ImGui::Button("short dialogue")) pdialogue->new_content("this is some short line");
	if (ImGui::Button("super short dialogue")) pdialogue->new_content("Địt mẹ");
	if (ImGui::Button("long dialogue")) pdialogue->new_content("(Stopping, breathless) Oh, Ivan! I was just thinking about the stories you told us yesterday. About the fairies and the trolls and the wise old owl. \
															Ivan: (Chuckling) And you believe all of that, do you?");
	ImGui::SliderFloat("dialogue speed", &pdialogue->_mspc, 0, 100);
	ImGui::SliderFloat("dialogue scale", &dialogue_scale, 0, 2);
	ImGui::Text("dialogue timer %d", pdialogue->_timer);
	
	ImGui::End();
	
	ImGui::Begin("Cam");
	
	ImGui::InputInt("time scale", &spms);
	ImGui::SliderFloat("cam zoom", &haven._cam._zoom, 0, 10);
	ImGui::SliderFloat("cam offsetx", &haven._cam._offset.x, -100, 100);
	ImGui::SliderFloat("cam offsety", &haven._cam._offset.y, -100, 100);
	ImGui::SliderFloat("cam speed", &haven._cam._smooth_speed, 0, 0.2);
	
	ImGui::End();
	
	ImGui::Begin("Character move");

	static std::vector<int> char_location (haven._world._characters.size(), 100);
	for (int i = 0; i != haven._world._characters.size(); i++)
	{
		ImGui::PushID(i);
		
		ImGui::PushItemWidth(100);
		ImGui::InputInt("char x location", &char_location[i]);
		ImGui::PopItemWidth();
		
		ImGui::SameLine();
		if (ImGui::Button("Go"))
		{
			haven._world.character_perform_queue(i, {MOVE_TO, mmath::fvec2{char_location[i] * 8.f, 16 * 8.f}});
		};

		ImGui::PopID();
	};

	ImGui::End();

	ImGui::Begin("Character dialogue");

	static std::vector<std::string> char_dialogues (haven._world._characters.size(), "DCMM");
	for (int i = 0; i != haven._world._characters.size(); i++)
	{
		ImGui::PushID(i);
		ImGui::PushItemWidth(200);
		ImGui::InputText("char dialogue", &char_dialogues[i]);
		ImGui::PopItemWidth();

		ImGui::SameLine();
		if (ImGui::Button("Talk"))
		{
			haven._world.character_perform_queue(i, {TALK, char_dialogues[i]});
		};

		ImGui::PopID();
	};

	ImGui::End();
	

	ImGui::Begin("Actions");

	for (auto& [ainfo, aresult] : haven._world._characters.get_component_vec<haven::character::action::record>())
	{
		auto& [atype, aparam] = ainfo;
		ImGui::Text("%s, %d, %s", haven::character::action::print_type(atype).c_str(), aparam.index(), haven::character::action::print_result(aresult).c_str());
	};
	ImGui::End();

#endif


	return true;
};


const std::unordered_map<int, int> KEYBOARD_ICON 
{
	{beaver::BTNU, 57},
	{beaver::BTND, 58},
	{beaver::BTNL, 55},
	{beaver::BTNR, 56},
	{beaver::BTNZ, 19},
	{beaver::BTNX, 20},
	{beaver::BTNC, 21},
	{beaver::BTNV, 22},
};

void draw1(haven::game& haven)
{
	SDL_Renderer* rdr = haven._sdl._sdl._renderer;
	const sdl::texture* tileset = haven._sdl._assets.get<sdl::texture>("tileset");
	auto textures = haven._sdl._assets.get_cvec<sdl::texture>();

	SDL_RenderSetScale(rdr, haven._cam._zoom, haven._cam._zoom);
	SDL_SetRenderDrawColor(rdr, 0,0,0,255);
	SDL_RenderClear(rdr);

	//sdl::draw_tilemap(rdr, haven._cam, haven._world._tilemap, haven._sdl._assets.get_cvec<sdl::texture>());
	
	// TODO:if outside, draw sky


	// Draw current location
	
	std::string current_player_location_name {*haven._world._characters
										.get_component<haven::character::location>(haven._player_cid)};

	const haven::location& current_player_location = haven._world._locations.at(current_player_location_name);

	// draw background
	sdl::draw_tilemap(rdr, haven._cam, current_player_location._background, textures);	
	
	// ground
	sdl::draw_tilemap(rdr, haven._cam, current_player_location._ground, textures);	
	

	// TODO:draw dialogue (if present)
	// Done for small talk
	for (auto [spr, dialogue]: haven._world._characters.view<beaver::sprite, haven::character::dialogue>())
	{
		sdl::font* terminess = haven._sdl._assets.get<sdl::font>("terminess");
		if (dialogue->is_active())
		{
			SDL_Surface* text_surface {TTF_RenderUTF8_Shaded_Wrapped(*terminess, dialogue->_content.c_str(),
						{255,255,255,255}, {0,0,0,150}, 300)};
			
			sdl::texture text {SDL_CreateTextureFromSurface(rdr, text_surface)};
			SDL_FreeSurface(text_surface);
			
			mmath::fvec2 text_dimension = {static_cast<float>(text._width) / haven._cam._zoom * dialogue_scale, 
											static_cast<float>(text._height)/ haven._cam._zoom * dialogue_scale};

			mmath::fvec2 spr_center_upper {spr->_rect.center() - mmath::fvec2{0, spr->_rect._size.y/2}};

			// textbox bottom border would be located 5 px above character head
												   // 3 //
			mmath::fvec2 text_bottom_border_center {spr_center_upper - mmath::fvec2{0, 3}};

			SDL_FRect text_rect {.x = text_bottom_border_center.x - text_dimension.x/2.f,
								 .y = text_bottom_border_center.y - text_dimension.y,
								 .w = text_dimension.x,
								 .h = text_dimension.y};
			
			sdl::drawdata text_drawdata {._tex = text,
										._src = {},
										._dst = text_rect};

			sdl::draw_with_cam(haven._cam, rdr, text_drawdata);
		};
	};


	// draw characters
	for (auto [spr, flags]: haven._world._characters.view<beaver::sprite, haven::character::flags>())
	{
		// TODO: handle flip flag
		unsigned fflag = flags->test(haven::character::FLAGS::FACING_LEFT) ? SDL_FLIP_HORIZONTAL : 0;
		
		sdl::drawdata cdrawdata {._tex = *tileset, 
								._src = tiled::rect_at(spr->_tileanimation.current_id(), haven._tileset),
								._dst = spr->_rect,
								._flipflags = fflag};
		sdl::draw_with_cam(haven._cam, rdr, cdrawdata); 
	};

	// draw keyboard (draw interaction key only)
	
		// drop movement key
	std::set<int> interaction_key = std::ranges::to<std::set<int>>(
					haven._player_control 
						| std::views::drop(4)
						| std::views::transform([](auto&& control){return control._input;}));
	mmath::ivec2 start_draw_pos {10, static_cast<int>(haven._cam._view._size.y) - 32 - 5};

	for (auto& input: interaction_key)
	{
		sdl::texture* keyboard_tex = haven._sdl._assets.get<sdl::texture>("ui_keyboard");
		sdl::font* terminess = haven._sdl._assets.get<sdl::font>("terminess");
		
		if (haven::available_player_control(haven._player_cid, haven._player_control, haven._world))
		
		SDL_SetTextureAlphaMod(*keyboard_tex, 100);

		sdl::drawdata keydrawdata {._tex = *keyboard_tex,
									._src = tiled::rect_at(KEYBOARD_ICON.at(input), 16, 12, 73),
									._dst = mmath::frect{start_draw_pos / haven._cam._zoom, mmath::ivec2{32,32} / haven._cam._zoom}};
		sdl::draw(rdr, keydrawdata);

		start_draw_pos -= {0,32 + 5};

	};


	// foreground
	sdl::draw_tilemap(rdr, haven._cam, current_player_location._foreground, textures);	

};

haven::game::game() :	_sdl("haven", 1280, 720, 60),
					_cam({._view = {0,0,600,600},
							._offset = {0,50},
							._zoom = 3})
{
	load_assets(_sdl._assets, _sdl._sdl._renderer);
	
	const sdl::texture* haven_chroma = _sdl._assets.get<sdl::texture>("tileset");

	_tileset._tilesize = 8;
	_tileset._numx = haven_chroma->_width / 8;
	_tileset._numy = haven_chroma->_height / 8;

	//auto load_player = [&](const tiled::tilemap& map)
	//{
	//	beaver::sprite spr;
	//	spr._rect = tiled::rect_at({100,16}, map);
	//	spr._tilcid = 4826;
	//};

	for (int i = 0; i != _world._characters.size(); i++)
	{
		std::string charname = _world._characters.get_component<character::info>(i)->_name;
		if (charname == "player")
		{
			_player_cid = i;
			break;
		};
	};


	using enum character::action::type;
	_player_control = {{  
						{beaver::BTNU, true, {JUMP, 4.f}},
						{beaver::BTND, true, {JUMP, 4.f}},
						{beaver::BTNL, true, {MOVE, -0.7f}},
						{beaver::BTNR, true, {MOVE, 0.7f}},
						{beaver::BTNZ, false,{OPEN_DOOR, {}}},
						{beaver::BTNX, false,{OPEN_DOOR, {}}},
						{beaver::BTNC, false,{OPEN_DOOR, {}}},
						{beaver::BTNV, false,{OPEN_DOOR, {}}},
					}};
	std::println();


	//auto& npc1 = _world._npcs.back();
	//npc1._routines.emplace([&](){return _world._clock.get_time()[1] > 1;}, 
	//		1, mmath::ivec2{10,0},
	//		[](haven::character& c, const int dtr)
	//		{
	//			std::println("npc1 {} barked", c._name);
	//			c._status._mood++;
	//			std::println("npc1 {} mood {}", c._name, c._status._mood);
	//		});

	// add event receiver
	_events._receivers.insert(&_world._event_handler);

};

void haven::game::run()
{
	//auto* bgm = _sdl._assets.get<sdl::music>("thenightislate");
	//Mix_PlayMusic(*bgm, -1);

	_sdl._updatef = [&](float dtr){return havened_update(*this, dtr);};
	_sdl._drawf = [&](){draw1(*this);};
	_sdl.run();
	

	//for (auto [locationname, _]: _world._locations)
	//{
	//	std::println("{}", locationname);
	//};

};

