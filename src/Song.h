#include <vector>
#include <string>
#include <iostream>

#include "VoiceLibrary.h"


class Song{
	/* The Song object stores all the information about a Song,
	** independent of the voice library that will be used to synthesize it.
	** It stores all the information extracted from a UST file.
	** It is roughly equivalent to sheet music.
	*/
	public:
		float tempo; //in BPM
		std::string projectName; // name of project
		std::string outFile; // default output file path
		std::string voiceDir; // default voice library location
		std::vector<Note> notes; // list of Notes that make up the song

		Song(std::string path); // read Song in from UST at path
		//std::vector<float> synthesize(voiceLibrary::VoiceLibrary library); TODO: Is this even useful?
		void synthesize(VoiceLibrary, std::string path); // synthesize song using a voicelibrary 
		                                                 // and write output to path
};
