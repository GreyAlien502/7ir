#include <vector>
#include <string>
#include <map>

#include "Phone.h"

class VoiceLibrary{
	private:
		int windowLength;
		std::vector<basePhone> phones;
		std::map<std::string,int> aliases;

		bool hasPhone(std::string alias);
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
