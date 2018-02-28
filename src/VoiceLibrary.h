#include <map>

#include "Phone.h"

class VoiceLibrary{
	/* This class stores the information required to use a voice library.
	*/
	private:
		int windowLength;
		std::vector<
			std::tuple<
				std::string, //path
				float, //offset
				float, //consonant
				float, //cutoff
				float, //preutter
				float  //overlap
			>
		> phones;
		std::map<std::string,int> aliases; // map from lyrics to index in phones

		// map from MIDI note numbers to prefix and suffix
		std::map<int,std::pair<std::string,std::string>> prefixMap;
		int maxNoteNum; //highest note number before using default high affixes
		int minNoteNum; //lowest note number before using default low affixes

		// Checks if there is a phone for an exact lyric
		bool hasPhone(std::string alias);
		// Makes .spch files to be read from during synthesis
		// A voicelibrary cannot be used without being compiled first
		void compile(std::string path);
		// add all samples from path/oto.ini to the library
		void importDir(std::string path);
		// add the proper affixes to the lyric based on the note it'll be synthesized at
		std::string affixedLyric(int noteNum, std::string lyric);

		// get string to check if sved .spch format matches this computer
		std::string getFormatString();
	public:
		float sampleRate;
		int hop;

		/* reads a new VoiceLibrary in:
		**	path: points to root directory containing oto.ini
		*/
		VoiceLibrary(
			std::string path,
			int overlap=16,
			int windowSize=2048,
			float sampleRate=44100);
		//returns a phone for a given note
		Phone getPhone(Note note);
};
