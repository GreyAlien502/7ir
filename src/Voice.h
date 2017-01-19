#include <vector>

#include "Sound.h"

class Voice{
	private:
		int windowLength;
		int hop;
		int hops;
		
		std::vector< std::vector<double> > magnitudes;
		std::vector< std::vector<double> > freqDisplacements;
		std::vector<double> frequencies;

		int getHop(double time);
	public:
		double sampleRate;
		double duration;
		Voice(Sound sample=Sound(),double freq=440);
		std::vector<double> synthesize();

		void stretch(double start, double end, double nuvolength);
		void amplify(double amplitude);
		void transpose(double frequency);
};
