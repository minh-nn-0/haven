#include <fstream>
#include <sstream>
#include <print>
int main()
{
	std::ifstream f {"characters"};

	std::string character;
	while (std::getline(f, character))
	{
		std::stringstream ss{character};
		std::string charname; int base_tileid;
		ss >> charname; ss >> base_tileid;

		std::println("{}, {}", charname, base_tileid);
	};

};
