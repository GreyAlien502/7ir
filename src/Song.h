#include <vector>
#include <string>

#include "VoiceLibrary.h"

class Song{
	public:
		double tempo;
		std::string projectName;
		std::string outFile;
		std::string voiceDir;
		std::vector<Note> notes;

		Song(std::string path);
		//std::vector<double> synthesize(voiceLibrary::VoiceLibrary library); TODO: Is this even useful?
		void synthesize(VoiceLibrary, std::string path);
};
