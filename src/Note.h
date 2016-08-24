#include <map>
#include <string>
#include <iostream>
#include <fstream>

std::map<std::string,std::string> parameters(std::ifstream& file);

class Note{
	public:
		std::string lyric;
		int notenum;
		double velocity;
		double delta;//in ms
		double duration;//in ms
		double length;//in ms

		Note(std::ifstream& file);
};
