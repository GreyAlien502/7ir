#include <vector>
#include <complex>

int samplesFromTime(double time);
double timeFromSamples(int samples);

class Sound{
	private:
		void lengthenVector(std::vector<std::vector<double> >& input, int start, int end, int nuvolength);
	public:
		int windowLength;
		int hop;
		int hops;
		std::vector<double> rawSynthesize();

		std::vector< std::vector<double> > magnitudes;
		std::vector< std::vector<double> > frequencies;

		double duration;
		int sampleRate;

		Sound(std::vector<double> pcm=std::vector<double>(0,0), int overlap=8, int windowSize=1024, int rate=44100);
		Sound compatibleSound(std::vector<double> pcm);
		std::vector<double> synthesize();
};
