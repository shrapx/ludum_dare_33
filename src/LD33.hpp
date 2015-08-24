#ifndef LD33_HPP_
#define LD33_HPP_

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include "SFML/System.hpp"
#include "SFML/Audio.hpp"

#include <jsoncpp/json/json.h>
#include "loader.hpp"

#include "timing.hpp"
#include "input.hpp"

#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923

using namespace std;
using namespace Json;

class Part
{
public:

	sf::Sprite sprite;
	sf::Vector2f m_offset;
	bool is_facing_right = true;

	/// todo from json parts {}
	enum : short{ NONE=0, VERT, HORIZ, CIRCLE};
	short m_movement;


	float m_tally = 0.0f;

	float m_walking = 1.0f;
	float m_idle = 0.5f;
	float m_distance = 1.0f;
	float m_speed = 1.0f;
	float m_phase = 0.0f;

	sf::Vector2f get_movement(bool is_moving = false)
	{
		m_tally += m_speed * (is_moving ? m_walking : m_idle);

		float value = (m_tally + m_phase*M_PI*2);
		sf::Vector2f out;

		switch (m_movement)
		{
		case VERT:
			{
				out.y = m_distance * sin(value);
				break;
			}
		case HORIZ:
			{
				out.x = m_distance * cos(value);
				break;
			}
		case CIRCLE:
			{
				out.x = m_distance * cos(value);
				out.y = m_distance * sin(value);
				break;
			}
		default:
			break;
		}
		return out;
	}
};

class Tile
{
public:
	int id =-1;

	/// ai hints
	bool jump_left=false; // adjacent and below is air : true, adjacent and above is true
	bool jump_right=false;

	bool walk_left=true; // adjacent tile is air : true
	bool walk_right=true;
	bool floor_below=false;
};

class Tiles
{
public:

	unordered_map<int, unordered_map<int, Tile>> m_map;

	void load( Json::Value& jval)
	{
		/// which layer is called "World"

		for ( auto& layer : jval["layers"])
		{
			if (layer["name"].asString() == "World")
			{
				auto& jdata = layer["data"];

				for (int num = 0; num < jdata.size(); ++num)
				{
					int x = num % 192;
					int y = num / 192;
					m_map[x][y].id = jdata[num].asInt()-1;
				}
				behaviours();
				return;
			}
		}
	}

	int getid(int x, int y) const
	{
		const auto& it = m_map.find(x);
		if ( it == m_map.end() ) return -1;
		const auto& map2 = it->second;
		const auto& it2 = map2.find(y);
		if ( it2 == map2.end() ) return 1;
		return it2->second.id;
	}

	const Tile* gett(int x, int y) const
	{
		const auto& it = m_map.find(x);
		if ( it == m_map.end() ) return new Tile();
		const auto& map2 = it->second;
		const auto& it2 = map2.find(y);
		if ( it2 == map2.end() ) return new Tile();
		return &it2->second;
	}

	void behaviours()
	{
		for (auto& x_pair : m_map)
		{
			int x = x_pair.first;
			auto& map_2 = x_pair.second;
			for (auto& y_pair : map_2)
			{
				int y = y_pair.first;
				Tile& tile = y_pair.second;

				tile.walk_left = ( getid(x-1, y) < 1 ) && (getid(x-1, y+1) > 0);
				tile.walk_right = ( getid(x+1, y) < 1 ) && (getid(x+1, y+1) > 0);
				tile.floor_below = ( getid(x, y+1) > 0 );
			}
		}

		for (auto& x_pair : m_map)
		{
			int x = x_pair.first;
			auto& map_2 = x_pair.second;
			for (auto& y_pair : map_2)
			{
				int y = y_pair.first;
				Tile& tile = y_pair.second;

				tile.jump_left  = satisfy_jump(x,y,0);
				tile.jump_right = satisfy_jump(x,y,1);

			}
		}
	}

	bool satisfy_jump(int x, int y, bool is_right)
	{
		/// 4,5,6
		int go_right = is_right ? 1 : -1;

		/// col first
		/// ##
		/// h#
		/// h#
		/// H~

		vector<sf::Vector2i> cpos =
		{ {0,-3 * go_right},
			{1, 1 * go_right},
			{1, 2 * go_right},
			{1, 3 * go_right} };

		for( sf::Vector2i& pos : cpos )
		{
			const Tile* a = gett(pos.x, pos.y);

			if (a->floor_below && a->id > 0)
			{
				return false;
			}
		}
		return true;
		/*
		for( int dist = 4; dist < 7; ++dist)
		{
			const Tile* a = gett(x-1, y+dist*go_right);

			bool jump_high = a->floor_below && a->id < 1;

			if (jump_high) return true;

			const Tile* b = gett(x-dist, y);

			bool jump_long = b->floor_below && b->id < 1;
			if (jump_long) return true;
		}
*/
	}

};


class Hero
{
public:

	enum : short { HEROES=0, MONSTERS, COINS};
	short m_type;

	enum : short { THINK, WAIT, MOVE, JUMP, DROP};
	enum : short { FACING_L=0, FACING_R };

	short m_facing = FACING_R;
	short m_state = MOVE;

	int wait_ticks = 0;
	int wait_trigger = 100;

	int jump_ticks = 0;

	int jump_tick_default = 20;
	int wait_ticks_default = 10;

	float jump_speed_y = 0.0f;
	float jump_speed_y_default = 0.2f;

	float jump_speed_x = 0.0f;
	float jump_speed_x_default = 1.0f;

	float move_speed = 1.2f;
	float move_fall = 2.0f;

	sf::Vector2f m_pos;
	sf::Vector2f m_origin;

	vector<vector<int>> m_parts;
	int m_part=0;

	bool has_enemy = false;
	int enemy_target;
	int enemy_x;

	bool has_path = false;
	sf::Vector2f path_target;
	int path_id = 0;

	sf::Vector2f m_path_pos;
	vector<sf::Vector2f> m_path;

	void update(Tiles& tiles)
	{
		int x = m_pos.x/8;
		int y = m_pos.y/8;

		Tile& tile = tiles.m_map[x][y];

		//cout << x << " "<< y << " " << tile.walk_right << endl;
		switch (m_state)
		{
		case WAIT:
			wait_ticks--;
			if (wait_ticks==0) m_state = THINK;
			break;

		case MOVE:
			if ( m_facing )
			{
				m_pos.x += move_speed;
			}
			else
			{
				m_pos.x -= move_speed;
			}
			break;

		case JUMP:
			if (jump_ticks>-jump_tick_default)
			{
				if (m_facing)
				{
					m_pos.x += jump_speed_x;
				}
				else
				{
					m_pos.x -= jump_speed_x;
				}

				m_pos.y -= jump_ticks*jump_speed_y;
				jump_ticks--;
			}
			else /// falling with damageable bounce
			{
				m_pos.y += move_fall;
			}
			break;

		case DROP: /// droping - no damage, when moving off an edge
			m_pos.y += move_fall;
			break;
		}

		/// prioritise dispatching of nearby enemies

		/*
		/// path following

		if (has_path)
		{
			if (path_id < m_path.size())
			{
				path_target = m_path_pos + m_path[path_id];

				float diff = path_target.x - m_pos.x;

				is_moving_right = diff > 0.0f;
				is_moving_left = !is_moving_right;


				/// distance to judge when to move to next node

				sf::Vector2f manhat =  path_target - m_pos;
				manhat.x = abs(manhat.x);
				manhat.y = abs(manhat.y);
				float dist = (manhat.x > manhat.y) ? manhat.x : manhat.y;

				//cout << path_target.x << " " << m_path_pos.x << " " << pos.x << " " << dist << endl;
				//cout << dist << endl;

				/// if its left, is_moving_left

				/// if its above, look for jump

				if (dist < 4.0f)
				{
					path_id++;
				}

			}
			else
			{
			  has_path = false;
			  path_id = 0;
			}

		}
*/
	}

	void disable_parts(unordered_map<int, Part>& parts)
	{
		int parts_id = m_state == JUMP;
		//for ( int part_id : m_parts[m_part] )
		for ( int part_id : m_parts[ parts_id ] )
		{
			Part& part = parts[part_id];
			part.sprite.setPosition( m_pos - m_origin
				+ part.m_offset + part.get_movement( m_state == MOVE ));
		}
	};

	void update_parts(unordered_map<int, Part>& parts)
	{
		int parts_id = m_state == JUMP;
		//for ( int part_id : m_parts[m_part] )
		for ( int part_id : m_parts[ parts_id ] )
		{
			Part& part = parts[part_id];

			part.sprite.setPosition( m_pos - m_origin
				+ part.m_offset + part.get_movement( m_state == MOVE ));
		}
	};

	void load_path(const string& name, Json::Value& jval) //"Heroes_Path"
	{
		for ( auto& layer : jval["layers"])
		{
			if (layer["name"].asString() == "AI")
			{
				for ( auto& object : layer["objects"])
				{
					if (object["name"].asString() == name)
					{
						m_path_pos.x = object["x"].asFloat();
						m_path_pos.y = object["y"].asFloat();
						auto& polyline = object["polyline"];
						m_path.resize( polyline.size() );

						for (int it=0; it<polyline.size(); ++it)
						{
							m_path[it] = sf::Vector2f( polyline[it]["x"].asFloat(),polyline[it]["y"].asFloat() );
						}
						return;
					}
				}
			}
		}
	}
};

class Think
{
public:

	/// choose a valid move
	void update(Hero& hero, Tile& tile)
	{
		if (hero.m_state != Hero::THINK) return;



		switch (hero.m_state)
		{
		case Hero::THINK:
		{
			think_random(hero, tile);
			break;
		}
		}
	}

	void update_nearest_type(int id, unordered_map<int, Hero>& m_heroes, short type)
	{
		Hero& hero = m_heroes[id];

		int min_dist = 8*16;
		int min_id = -1;
		int min_x = 0;

		for ( auto& itpair : m_heroes )
		{
			int   it_id = itpair.first;
			Hero& it_hero = itpair.second;
			if (id == it_id) continue;

			if (hero.m_type == type)
			{
				float dist = abs( hero.m_pos.x - it_hero.m_pos.x );
				if (dist < min_dist)
				{
					min_id = id;
					min_x = it_hero.m_pos.x;
				}
			}
		}

		hero.enemy_target = min_id;
		hero.enemy_x = min_x;
	}

	void think_the_hero(Hero& hero, Tile& tile)
	{
		hero.m_facing = 1;

		// enemies ?
		/// target enemy

		/// set as target

		// coins ?

		if (tile.walk_right) // can walk right?
		{
			hero.m_state = Hero::MOVE;
		}
		else if (tile.jump_right) // can jump right?
		{
			hero.m_state = Hero::JUMP;
		}

		/// random choice
		bool to_walk = rand() % 2;
		bool to_jump = !to_walk;

		bool to_left = rand() % 2;
		bool to_right = !to_left;


		if (to_right)
		{
			if (to_walk && tile.walk_right)
			{
				hero.m_state = Hero::MOVE;
			}
			else if (to_jump && tile.jump_right)
			{
				hero.m_state = Hero::JUMP;
			}
		}
		else
		{
			if (to_walk && tile.walk_left)
			{
				hero.m_state = Hero::MOVE;
			}
			else if (to_jump && tile.jump_left)
			{
				hero.m_state = Hero::JUMP;
			}
		}

		if (hero.m_state == Hero::JUMP)
		{
			hero.jump_ticks = hero.jump_tick_default;
			hero.jump_speed_x = hero.jump_speed_x_default;
			hero.jump_speed_y = hero.jump_speed_y_default;
		}

		if (hero.m_state == Hero::THINK)
		{
			/// wait
			hero.m_state = Hero::WAIT;
			hero.wait_ticks = hero.wait_ticks_default + (rand() % hero.wait_ticks_default);
		}

		if (hero.m_state != Hero::THINK) cout << hero.m_state << endl;

	}

	void think_random(Hero& hero, Tile& tile)
	{
		/// random choice
		bool to_walk = rand() % 2;
		bool to_jump = !to_walk;

		bool to_left = rand() % 2;
		bool to_right = !to_left;

		hero.m_facing = to_right;

		if (to_right)
		{
			if (to_walk && tile.walk_right)
			{
				hero.m_state = Hero::MOVE;
			}
			else if (to_jump && tile.jump_right)
			{
				hero.m_state = Hero::JUMP;
			}
		}
		else
		{
			if (to_walk && tile.walk_left)
			{
				hero.m_state = Hero::MOVE;
			}
			else if (to_jump && tile.jump_left)
			{
				hero.m_state = Hero::JUMP;
			}
		}

		if (hero.m_state == Hero::JUMP)
		{
			hero.jump_ticks = hero.jump_tick_default;
			hero.jump_speed_x = hero.jump_speed_x_default;
			hero.jump_speed_y = hero.jump_speed_y_default;
		}

		if (hero.m_state == Hero::THINK)
		{
			/// wait
			hero.m_state = Hero::WAIT;
			hero.wait_ticks = hero.wait_ticks_default + (rand() % hero.wait_ticks_default);
		}

		if (hero.m_state != Hero::THINK) cout << hero.m_state << endl;

	}
};

class Collision
{

public:
	//unordered_set<int> is_overlap;

	update(Hero& hero, Tile& tile)
	{
		/*
		/// if in a tile
		if (tile.id > 0)
		{
			/// check above
			float t_floor = floor(hero.m_pos.y/8)*8;
			hero.m_pos.y = t_floor+6;

			/// check sides

		}*/

		switch (hero.m_state)
		{
		case Hero::JUMP:
		case Hero::DROP:
		{
			if (tile.floor_below)
			{
				/// is next movement going to put us through the floor?
				float t_floor = floor(hero.m_pos.y/8)*8;
				float nt_floor = floor((hero.m_pos.y+hero.move_fall)/8)*8;
				bool through_floor = (t_floor != nt_floor);

				if (through_floor)
				{
					/// correct y position
					hero.m_pos.y = t_floor+6;

					/// stop jumping
					hero.m_state = Hero::WAIT;
					hero.wait_ticks = hero.wait_ticks_default;
				}
			}
			break;
		}
		case Hero::MOVE:
		{
			/// if in a tile lets warp above it:
			if (tile.id < 1)
			{
				float t_floor = floor(hero.m_pos.y/8)*8;
				hero.m_pos.y = t_floor+6;
			}

			if ( (hero.m_facing && !tile.walk_right) || (!hero.m_facing && !tile.walk_left) )
			{
				hero.m_state = Hero::WAIT;
				hero.wait_ticks = hero.wait_ticks_default;
			}
			break;
		}
		}
	}
};

class LD33
{
public:

	LD33() : input(m_commands)
	{
		window.create(sf::VideoMode(800, 600),
			"LD33 - shrapx", sf::Style::Titlebar | sf::Style::Close);

		window.setVerticalSyncEnabled(true);
		window.setKeyRepeatEnabled(false);

		render_texture.create(800 * zoom_factor, 600 * zoom_factor, false);
		//render_texture.create(400 * zoom_factor, 300 * zoom_factor, false);
		render_sprite.setTexture(render_texture.getTexture());
		render_sprite.setScale(1/zoom_factor,1/zoom_factor);

		//background_sprite.setTexture(background_texture.getTexture());
		//background_sprite.setScale(1/zoom_factor,1/zoom_factor);

		view = render_texture.getDefaultView();

		//view.setCenter(0,0);
		view.setCenter(150*8,24*8);

	};

	void config(const string& filename = "data/config.json")
	{

		// load config
		Value config_json = Loader::load(filename);

		// keyboard mouse input
		Value& binds = config_json["binds"];

		for(ValueIterator it=binds.begin(); it != binds.end(); it++ )
		{
			string key = it.key().asString();
			string cmd = (*it).asString();
			for (unsigned short i=0; i<keys.size(); ++i)
			{
				string ikey = keys.at(i);
				if (ikey == key )
				{
					cout << " key found " << key << endl;
					input.key_cmd[i] = cmd;
					m_commands[0][cmd] = 0;
					break;
				}
			}
		}

		/// textures
		for (string tex_name : config_json["textures"].getMemberNames() )
		{

			string tex_file = config_json["textures"][tex_name].asString();

			cout << tex_name << ":" << tex_file << endl;

			m_textures.emplace(tex_name, make_shared<sf::Texture>());

			auto& tex = m_textures[tex_name];

			if ( tex->loadFromFile(tex_file) )
			{
				cout << "texture success." << endl;
			}
			else cout << "texture failed." << endl;
			//*/
		}

		/// generate background
		//sf::Sprite sprite;
		//sprite.setTexture( *m_textures.at("tileset_01") );
		//sprite.setTextureRect( sf::IntRect(88,0,8,8) );
		//render_background.draw( render_sprite );

		tilesprite.setTexture( *m_textures.at("tileset_01") );
		//tilesprite.setOrigin(4,4);

		/// load parts
		for (string& name : config_json["parts"].getMemberNames() )
		{
			auto& jpart = config_json["parts"][name];
			int id = new_id(name);

			/// Parts
			m_parts.emplace(id, Part());
			Part& part = m_parts[id];

			/// set texture
			part.sprite.setTexture( *m_textures.at("tileset_01") );


			/// tex rect and origin
			string move_text = jpart["move"].asString();

			if (move_text == "vert") part.m_movement = Part::VERT;
			if (move_text == "horiz") part.m_movement = Part::HORIZ;
			if (move_text == "circle") part.m_movement = Part::CIRCLE;

			auto& jpos = jpart["offset"];
			part.m_offset = sf::Vector2f(jpos[0].asInt(), jpos[1].asInt());

			auto& jrect = jpart["coord"];
			auto rect = sf::IntRect(jrect[0].asInt(), jrect[1].asInt(), jrect[2].asInt(), jrect[3].asInt());

			auto& jori = jpart["origin"];
			auto origin = sf::Vector2f(jori[0].asInt(), jori[1].asInt());

			part.sprite.setTextureRect( rect );
			part.sprite.setOrigin( origin );

			auto& jstat = jpart["stat"];
			part.m_walking = jstat[0].asFloat();
			part.m_idle = jstat[1].asFloat();
			part.m_distance = jstat[2].asFloat();
			part.m_speed = jstat[3].asFloat();
			part.m_phase = jstat[4].asFloat();
		}
		//*/

		/// load blueprints
		for (const string& name : config_json["blueprints"].getMemberNames() )
		{
			auto& blueprint = config_json["blueprints"][name];

			int id = new_id(name);

			/// hero ent is a group of several Parts
			//m_heroes.emplace(id, Hero());
			Hero& hero = m_heroes[id];

			hero.m_origin.x = blueprint["origin"][0].asFloat();
			hero.m_origin.y = blueprint["origin"][1].asFloat();

			/// get part ids in draw order

			hero.m_parts.resize(2);
			load_parts(blueprint["walk"], hero.m_parts[0]);
			load_parts(blueprint["jump"], hero.m_parts[1]);
		}

		/// load spawns

		/// assign kvm to a blueprint

		string kvm_name = config_json["kvm"].asString();

		input.kvm_id = m_names[kvm_name];

	};


	void load_parts(const Value& jparts, vector<int>& parts )
	{
		for (auto& jpart_name : jparts )
		{
			string part_name = jpart_name.asString();
			int part_id = m_names[part_name];
			//cout << part_name << ":" << part_id << endl;

			parts.push_back(part_id);
		}
	};


	int run()
	{
		config( _WIN64 ? "data\\config.json" : "data/config.json" );

		{
			Json::Value tiles_json = Loader::load("data/untitled.json");
			m_tiles.load(tiles_json);

			/// get polyline
			int id = input.kvm_id;

			auto& hero = m_heroes[id];

			/// give hero think and collision
			m_thinks[id];
			m_collisions[id];

			hero.load_path("heroes_path", tiles_json);
			hero.path_target = hero.m_path_pos + hero.m_path[0];
			hero.has_path = true;

			/// start hero at first target
			hero.m_pos = hero.path_target; /// m_pos set ok here
		}

		bool game_over = false;

		while(!game_over)
		{
			if ( timing.update() )
			{
				// events here?
				update();
			}
			else
			{
				render();
			}
			game_over = input.events(window);
		}

		window.close();
		return 0;
	}

private:

	Timing timing;

	sf::RenderWindow window;
	sf::View view;

	float zoom_factor = 1/4.0f;
	sf::Sprite render_sprite;
	sf::RenderTexture render_texture;

	sf::Sprite tilesprite;

	//sf::Sprite background_sprite;
	//sf::RenderTexture background_texture;

	unordered_map<string, shared_ptr<sf::Texture>> m_textures;

	/// world contents

	Tiles m_tiles;

	/// entity components:

	/// id
	int m_counter = 0;
	unordered_set<int> m_ids; /// entity ids

	/// names lookup id by name
	unordered_map<string, int> m_names;

	/// very dumb sub-sprite
	unordered_map<int, Part> m_parts;

	/// Heros = what is heron of the entity, and shape! (todo texture)


	unordered_map<int, Hero> m_heroes;
	// Hears = sound events? (todo ogg)

	/// Collision = physics, overlap/collision detect, falling,
	unordered_map<int, Collision> m_collisions;

	// collision
	/// Commands = keyboard or ai input

	unordered_map<int, unordered_map<string, bool>> m_commands;

	/// Moves = actions

	/// Thinks = ai

	unordered_map<int, Think> m_thinks;

	/// future action based on present and past state


	Input input;


	int new_id(const string& name)
	{
		m_counter++;

		m_ids.insert(m_counter);

		m_names.emplace(name, m_counter);

		return m_counter;
	}

	bool get_command(int id, const string& cmd)
	{
		return m_commands[id][cmd];
	}

  void update()
  {

  	//view.setCenter( m_heroes[input.kvm_id].sprite.getPosition() );

  	{
			int id = input.kvm_id;

			bool up = get_command(id,"up");
			bool dw = get_command(id,"down");
			bool le = get_command(id,"left");
			bool ri = get_command(id,"right");

			const sf::Vector2f& center = view.getCenter();
			view.setCenter( sf::Vector2f( center.x + (ri - le)*2, center.y + (dw - up)*2 ) );
  	}

		for ( auto& itpair : m_collisions)
		{
			int id = itpair.first;

			Collision& col = itpair.second;
			Hero& hero = m_heroes[id];

			int x = hero.m_pos.x/8;
			int y = hero.m_pos.y/8;
			Tile& tile = m_tiles.m_map[x][y];
			col.update( hero, tile );
		}

		for ( auto& itpair : m_thinks)
		{
			int id = itpair.first;

			Think& think = itpair.second;
			Hero& hero = m_heroes[id];

			if (hero.m_type == Hero::HEROES)
			{
				think.update_nearest_type(id, m_heroes, Hero::MONSTERS);
			}

			int x = hero.m_pos.x/8;
			int y = hero.m_pos.y/8;
			Tile& tile = m_tiles.m_map[x][y];
			think.update( hero, tile );
		}

		for ( auto& itpair : m_heroes)
		{
			int id = itpair.first;
			Hero& hero = itpair.second;

			hero.update(m_tiles);
			hero.update_parts(m_parts);
		}
  };

  void render()
  {
		//window.setView(view);
		//window.clear(sf::Color::Black);

		render_texture.setView(view);
		//render_texture.clear(sf::Color::Black);
		render_texture.clear(sf::Color(8,96,128));


		//float ipo = timing.get_interpolation();

		/// tiles function

		const sf::Vector2f& center = view.getCenter();
		int vx = center.x / 8;
		int vy = center.y / 8;
		int rangex = 14;
		int rangey = 11;

		//cout << vx << " " << vy << endl;

		for (int tx = vx-rangex; tx < (vx+rangex); ++tx)
		{
			auto& _map = m_tiles.m_map[tx];

			for (int ty = vy-rangey; ty < (vy+rangey); ++ty)
			{
				const auto& tile = _map[ty];
				int tile_pos = tile.id;
				if (tile_pos > 0)
				{
					int tile_x = tile_pos % 16;
					int tile_y = tile_pos / 16;

					tilesprite.setTextureRect( sf::IntRect( tile_x*8, tile_y*8, 8, 8) );
					tilesprite.setPosition( tx * 8, ty * 8);

					sf::Color col = sf::Color::White;

					if (tile.walk_left) col = sf::Color::Red;
					if (tile.walk_right) col = sf::Color::Blue;

					//if (tile.jump_left) col = sf::Color::Cyan;
					//if (tile.jump_right) col = sf::Color::Yellow;

					tilesprite.setColor(col);

					render_texture.draw(tilesprite);
				}
			}
		}

		/// draw characters

		for (const auto& itpair : m_heroes)
		{
			int id = itpair.first;
			const Hero& hero = itpair.second;

			int parts_id = hero.m_state == Hero::JUMP;
			//for ( int part_id : hero.m_parts[hero.m_part] )
			for ( int part_id : hero.m_parts[parts_id] )
			{
				const Part& part = m_parts[part_id];

				render_texture.draw(part.sprite);
			}
		}

		/// draw fg

		/// draw ui

		//auto ui = view.getCenter();
		//window.draw(text);

		//window.display();

		render_texture.display();

		window.draw( render_sprite );
		window.display();
  };
};

#endif
