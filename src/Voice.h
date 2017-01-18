#include <vector>

#include "Sound.h"

class Voice{
	private:
		double sampleRate;
		int windowLength;
		int hop;
		int hops;
		double duration;
		
		std::vector< std::vector<double> > magnitudes;
		std::vector< std::vector<double> > freqDisplacements;
		std::vector<double> frequencies;

		int getHop(double time);
	public:
		Voice(Sound sample,double freq);
		std::vector<double> synthesize();

		void stretch(double start, double end, double nuvolength);
		void amplify(double amplitude);
		void transpose(double frequency);
};
