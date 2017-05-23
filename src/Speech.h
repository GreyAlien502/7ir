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
		std::vector< std::vector<double> > magnitudes;
		std::vector< std::vector<double> > freqDisplacements;
		std::vector<double> frequencies;

		//helper methods/data
		std::vector<double> remainder;
		Sound toSound(int endHop);
	public:
		//public knowledge
		double sampleRate;
		double duration;

		//creation, addition, & destruction
		Speech(Sound sample=Sound(), double frequency=440);
		void add(Speech addee, double overlap);
		Sound startToSound(double endTime);
		void crop(double starttime, double endtime);
		std::vector<double> pop(double popLength);
		std::vector<double> synthesize();

		//modifying
		void stretch(double start, double end, double nuvolength);
		void amplify(std::function<double (double)> freqs);
		void transpose(std::function<double (double)> frequency, double endTime);

		//for reading & writing to disc
		Speech(std::istream& filestream);
		void write(std::ostream& filestream);
};
