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

float mag(sf::Vector2f& vec)
{
	return std::sqrt(std::pow(vec.x, 2) + std::pow(vec.y, 2));
}
sf::Vector2f norm(sf::Vector2f& vec)
{
	return vec / mag(vec);
}

class Hero
{
public:

	sf::Sprite sprite;

	vector<vector<int>> m_parts;
	int m_part=0;

	bool is_facing_right = true;

	bool is_moving = false;

	bool is_jumping = false;

	bool has_enemy = false;
	int enemy_target;

	bool has_path = false;
	sf::Vector2f path_target;
	int path_progress = 0;

	void update()
	{
		if(has_enemy)
		{
			/// walk toward + bounce enemy

		}
		else if(has_path)
		{
			/// walk toward path target
			auto pos = sprite.getPosition();
			auto dif = pos - path_target ;
			auto dir = norm( dif );
			dir.x *= 0.5f;
			dir.y *= 0.5f;
			sprite.setPosition( pos + dir );
		}
	}

};

class Tiles
{
public:

	unordered_map<int, unordered_map<int, int>> m_map;

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
					m_map[x][y] = jdata[num].asInt()-1;

				}
				return;
			}
		}
	}

	sf::IntRect draw_range()
	{
		/// x,y and width and height

	}
};

class Matter
{

public:

	bool is_falling = 1;

	unordered_set<int> is_overlap;

	update()
	{
		// update is falling
		///is_falling = 0;

		// update touching an enemy
		///is_overlap = get_overlaps();

		//
	}
};

class Think
{
public:

	/// make positional entity out of paths, with a target to next one and previous

	sf::Vector2f position;
	vector<sf::Vector2f> m_path;

	void update(Hero& hero)
	{
		/// path following

		if (hero.has_path)
		{
			if (hero.path_progress < m_path.size())
			{
				hero.path_target = position + m_path[hero.path_progress];
				sf::Vector2f manhat = hero.sprite.getPosition() - hero.path_target;
				manhat.x = abs(manhat.x);
				manhat.y = abs(manhat.y);
				float dist = manhat.x > manhat.y ? manhat.x : manhat.y;

				cout << dist << endl;

				if (dist < 4.0f)
				{
					hero.path_progress++;
				}
				hero.is_moving = true;
			}
			else
			{
				hero.is_moving = false;
			  hero.has_path = false;
			  hero.path_progress = 0;
			}
		}

	}

	void load(Json::Value& jval)
	{
		for ( auto& layer : jval["layers"])
		{
			if (layer["name"].asString() == "AI")
			{
				for ( auto& object : layer["objects"])
				{
					if (object["name"].asString() == "Heroes_Path")
					{
						position.x = object["x"].asFloat();
						position.y = object["y"].asFloat();
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
			m_heroes.emplace(id, Hero());
			Hero& hero = m_heroes[id];

			hero.sprite.setOrigin(blueprint["origin"][0].asInt(), blueprint["origin"][1].asInt());

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
			m_thinks[id].load(tiles_json);

			m_heroes[id].path_target = m_thinks[id].position + m_thinks[id].m_path[0];
			m_heroes[id].sprite.setPosition( m_heroes[id].path_target );

			m_heroes[id].has_path = true;
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

	/// Matter = physics, overlap/collision detect, falling,
	unordered_map<int, Matter> m_matters;
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

		for ( auto& itpair : m_thinks)
		{
			int id = itpair.first;

			Think& think = itpair.second;
			think.update( m_heroes[id] );
		}

		for ( auto& itpair : m_heroes)
		{
			int id = itpair.first;
			Hero& hero = itpair.second;

			hero.update();

			/// which parts list
			if (hero.is_jumping)
			{
				update_see(hero, hero.m_parts[1]);
			}
			else
			{
				update_see(hero, hero.m_parts[0]);
			}
			hero.m_part = hero.is_jumping;

		}
  };

	void update_see(Hero& hero, vector<int>& parts)
	{
		for ( int part_id : parts )
		{
			Part& part = m_parts[part_id];

			part.sprite.setPosition( hero.sprite.getPosition() - hero.sprite.getOrigin()
				+ part.m_offset + part.get_movement(hero.is_moving));
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
				int tile_pos = _map[ty];
				if (tile_pos > 0)
				{
					int tile_x = tile_pos % 16;
					int tile_y = tile_pos / 16;

					tilesprite.setTextureRect( sf::IntRect( tile_x*8, tile_y*8, 8, 8) );

					tilesprite.setPosition( tx * 8, ty * 8);

					render_texture.draw(tilesprite);
				}
			}
		}

		/// draw characters

		for ( auto& itpair : m_heroes)
		{
			int id = itpair.first;
			Hero& hero = itpair.second;

			for ( int part_id : hero.m_parts[hero.m_part] )
			{
				Part& part = m_parts[part_id];

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
