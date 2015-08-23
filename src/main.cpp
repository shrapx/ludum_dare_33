
#include <string>
#include "LD33.hpp"


int main(int argc, char **argv) {

	std::string path = _WIN64 ?  "data\\config.json" : "data/config.json";

	LD33 a;

	//a._config( argc>1 ? argv[1] : path );

  return a.run();
}
