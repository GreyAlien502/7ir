#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>


#include "VoiceLibrary.h"

namespace song{
	std::map<std::string,std::string> parameters(std::ifstream& file);
	int timeToSamples(double time);
	double samplesToTime(int samples);

	class Note{
		public:
			std::string lyric;
			int notenum;
			double velocity;
			double delta;//in ms
			double duration;//in ms
			double length;//in ms

			Note(std::ifstream& file);
			sound::Sound getSound(voiceLibrary::VoiceLibrary);
			voiceLibrary::Phone getPhone(voiceLibrary::VoiceLibrary);
	};
	class Song{
		public:
			double tempo;
			std::string projectName;
			std::string outFile;
			std::string voiceDir;
			std::vector<Note> notes;

			Song(std::string path);
			//std::vector<double> synthesize(voiceLibrary::VoiceLibrary library); TODO: Is this even useful?
			void synthesize(voiceLibrary::VoiceLibrary, std::string path);
	};
}
