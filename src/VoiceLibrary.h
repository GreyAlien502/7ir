#include <vector>
#include <string>
#include <map>

#include "Phone.h"

class VoiceLibrary{
	private:
		int windowLength;
		std::vector<
			std::tuple<
				std::string,
				double, //offset
				double, //consonant
				double, //cutoff
				double, //preutter
				double  //overlap
			>
		> phones;
		std::map<std::string,int> aliases;

		std::map<int,std::pair<std::string,std::string>> prefixMap;
		int maxNoteNum;
		int minNoteNum;

		bool hasPhone(std::string alias);
		void compile(std::string path);
		void importDir(std::string path);
		std::string affixedLyric(int noteNum, std::string lyric);

		std::string getFormatString();
	public:
		int sampleRate;
		int hop;
		VoiceLibrary(
			std::string path,
			int overlap=16,
			int windowSize=2048,
			int sampleRate=44100);
		Phone getPhone(Note note);
};
