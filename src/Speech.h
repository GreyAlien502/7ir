#include <vector>
#include <iostream>

#include "Sound.h"

class Speech{
	private:
		int windowLength;
		int hop;
		int hops;
		
		std::vector< std::vector<double> > magnitudes;
		std::vector< std::vector<double> > freqDisplacements;
		std::vector<double> frequencies;

		std::vector<double> synthesize(int hops);
		std::vector<double> remainder;

		int getHop(double time);
	public:
		double sampleRate;
		double duration;

		Speech(Sound sample=Sound(), double frequency=440);
		std::vector<double> synthesize();
		std::vector<double> pop(double popLength);

		Speech(std::istream& filestream);
		void write(std::ostream& filestream);

		void stretch(double start, double end, double nuvolength);
		void amplify(double amplitude);
		void transpose(double frequency);

		void add(Speech adee, double overlap);
};
