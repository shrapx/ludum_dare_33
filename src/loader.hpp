/*
 * Loader.hpp
 *
 *  Created on: 14 Sep 2014
 *      Author: shrapx
 */

#ifndef LOADER_HPP_
#define LOADER_HPP_

#include <iostream>
#include <fstream>
#include <sstream>

#include <jsoncpp/json/json.h>


class Loader
{
public:
	static void load(const std::string& filename, Json::Value& content)
	{
		std::cout << filename << ": loading" << std::endl;

		std::ifstream t(filename.c_str());
		if ( t.fail() )
		{
			std::cout << filename << ": load fail" << std::endl;
			return;
		}

		std::stringstream buffer;
		buffer << t.rdbuf();

		Json::Reader reader;
		bool parse = reader.parse(buffer.str(), content );

		if ( !parse )
		{
			// report the failure and the locations in the document.
			std::cout  << "Failed to parse configuration\n"
								 << reader.getFormattedErrorMessages();
		}
	}

	static Json::Value load(const std::string& filename)
	{
		Json::Value content;
		load(filename, content);
		return content;
	}
};


#endif /* LOADER_HPP_ */
