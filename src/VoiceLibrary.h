#include <vector>
#include <string>
#include <map>

#include "Sound.h"

namespace voiceLibrary{
	class Phone{
		public:
			int consonant; //all in hops
			int preutter;
			int overlap;
			double frequency;
			sound::Sound sample;

			Phone(std::vector<double> pcm = std::vector<double>(2048,0),
				double consonantTime=0, double preutterTime=0, double overlapTime=0,
				int windowOverlap=16, int windowSize=2048, int sampleRate=44100);
			sound::Sound note(int notenum, double length);
	};
	class VoiceLibrary{
		private:
			std::vector<Phone> phones;
			std::map<std::string,int> aliases;
		public:
			int sampleRate;
			int windowLength;
			int hop;
			VoiceLibrary(std::string path, int overlap=16, int windowSize=2048, int sampleRate=44100);
			bool hasPhone(std::string alias);
			Phone getPhone(std::string alias);
	};
}
