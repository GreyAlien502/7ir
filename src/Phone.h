#include <vector>
#include <iostream>

#include "Speech.h"
#include "Note.h"

class Phone{
	protected:
	public:
		float consonant; //all in seconds
		float preutter;
		float overlap;
		Speech sample;

		Phone(
			std::vector<float> pcm,
			float consonantTime, float preutterTime, float overlapTime,
			int windowOverlap, int windowSize, int sampleRate
		);
		Phone(
			Speech soundSample,
			float consonantTime, float preutterTime, float overlapTime
		);
		Phone( int windowOverlap=16, int windowSize=2048, int sampleRate=44100);

		float getConsonant();
		float getPreutter();
		float getOverlap();

		Phone(std::istream& filestream);
		void write(std::ostream& filestream);
};
