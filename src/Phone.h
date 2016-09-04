#include <vector>

#include "Sound.h"
#include "Note.h"

class Phone{
	private:
		double frequency;
	public:
		int consonant; //all in hops
		int preutter;
		int overlap;
		Sound sample;

		Phone(std::vector<double> pcm = std::vector<double>(2048,0),
			double consonantTime=0, double preutterTime=0, double overlapTime=0,
			int windowOverlap=16, int windowSize=2048, int sampleRate=44100);
		Phone adjustPhone(Note& note);
		double getConsonant();
		double getPreutter();
		double getOverlap();
};
