#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "VoiceLibrary.h"

std::map<std::string,std::string> parameters(std::ifstream& file);

class Song{
	public:
		float tempo;
		std::string projectName;
		std::string outFile;
		std::string voiceDir;
		std::vector<Note> notes;

		Song(std::string path);
		//std::vector<float> synthesize(voiceLibrary::VoiceLibrary library); TODO: Is this even useful?
		void synthesize(VoiceLibrary, std::string path);
};
