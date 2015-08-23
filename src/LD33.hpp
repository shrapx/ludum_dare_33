#ifndef LD33_HPP_
#define LD33_HPP_

#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include "SFML/System.hpp"
#include "SFML/Audio.hpp"

#include <jsoncpp/json/json.h>
#include "loader.hpp"

#include "timing.hpp"
#include "input.hpp"

class See
{
public:

	sf::Sprite sprite;

	bool is_facing_right = true;
	bool is_moving = false;
	float walk_frame = 0;

  int anim_frames = 0;

	int frame_x = 0;
	int frame_y = 0;

	int target;

	void update(float time_step)
	{
		if (is_moving)
		{
			walk_frame = fmod(walk_frame + time_step * 6.0f, anim_frames);

			int frame_x = int( walk_frame ) * 32;     /// anim_frames high

			int frame_y = int( is_facing_right ) * 32; /// 2 frames wide: 0 or 1 / left or right

			sprite.setTextureRect( sf::IntRect(frame_x, frame_y, 32, 32) );
		}
		else
		{
			sprite.setTextureRect( sf::IntRect(0, 0, 32, 32) );
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

		window.setKeyRepeatEnabled(false);
	};

	int run()
	{
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

	/// world contents

	/// entity components:

	/// id
	unordered_set<int> m_ids; /// entity ids

	/// names lookup id by name
	unordered_map<string, int> m_name;

	/// Sees = what is seen of the entity (todo texture)
	unordered_map<int, See&> m_sees;

	/// Controls = keyboard or ai input

	unordered_map<int, unordered_map<string, bool>> m_commands;

	/// Moves = actions

	/// Thinks = ai

	/// Hears = sound events (todo ogg)


	Input input;

	bool get_command(int id, const string& cmd)
	{
		return m_commands[id][cmd];
	}

  void update()
  {

  };

  void render()
  {
		window.setView(view);
		window.clear(sf::Color::Black);

		//float ipo = timing.get_interpolation();

		/// draw bg

		/// draw characters

		/// draw fg

		/// draw ui

		//auto ui = view.getCenter();
		//window.draw(text);

		window.display();
  };
};

#endif
