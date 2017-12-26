#include <vector>
#include <iostream>
#include <functional>

#include "Sound.h"

class Speech{
	private:
		//timing information
		int windowLength;
		int hop;
		int hops;
		
		//time-based data
		std::vector< std::vector<float> > magnitudes;
		std::vector< std::vector<float> > freqDisplacements;
		std::vector<float> frequencies;

		//helper methods/data
		std::vector<float> remainder;
		Sound toSound(int endHop);
	public:
		//public knowledge
		float sampleRate;
		float duration;

		//creation, addition, & destruction
		Speech(Sound sample=Sound());
		void add(Speech addee, float overlap);
		Sound startToSound(float endTime);
		void crop(float starttime, float endtime);
		std::vector<float> pop(float popLength);
		std::vector<float> synthesize();

		//modifying
		void stretch(float start, float end, float nuvolength);
		void amplify(std::function<float (float)> freqs);
		void transpose(std::function<float (float)> frequency, float endTime);

		//for reading & writing to disc
		Speech(std::istream& filestream);
		void write(std::ostream& filestream);
};
