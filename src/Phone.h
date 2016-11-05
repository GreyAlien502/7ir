#include <vector>

#include "Sound.h"
#include "Note.h"

class Phone{
	protected:
		int consonant; //all in hops
		int preutter;
		int overlap;
	public:
		Sound sample;

		Phone(int consonant=0, int preutter=0, int overlap=0,
			Sound sample=Sound());
		double getConsonant();
		double getPreutter();
		double getOverlap();
};

class basePhone: public Phone{
	private:
		double frequency;

	public:
		basePhone(std::vector<double> pcm = std::vector<double>(2048,0),
			double consonantTime=0, double preutterTime=0, double overlapTime=0,
			int windowOverlap=16, int windowSize=2048, int sampleRate=44100);
		Phone adjustPhone(Note& note, double tempo);
};
