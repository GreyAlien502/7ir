#include <vector>
#include <iostream>

#include "Speech.h"
#include "Note.h"

class Phone{
	protected:
	public:
		double consonant; //all in seconds
		double preutter;
		double overlap;
		Speech sample;

		Phone(
			std::vector<double> pcm,
			double consonantTime, double preutterTime, double overlapTime,
			int windowOverlap, int windowSize, int sampleRate);
		Phone( int windowOverlap=16, int windowSize=2048, int sampleRate=44100);

		double getConsonant();
		double getPreutter();
		double getOverlap();

		Phone(std::istream& filestream);
		void write(std::ostream& filestream);
};
