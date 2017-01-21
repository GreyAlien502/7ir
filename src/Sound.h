#include <vector>
#include <complex>

int samplesFromTime(double time);
double timeFromSamples(int samples);

class Sound{
	private:
		int getHop(double time);
		void lengthenVector(std::vector<std::vector<double> >& input, int start, int end, int nuvolength);
	public:
		int windowLength;
		int overlap;
		int hop;
		int hops;

		std::vector< std::vector<double> > magnitudes;
		std::vector< std::vector<double> > frequencies;

		double duration;
		int sampleRate;

		Sound(std::vector<double> pcm=std::vector<double>(0,0), int overlap=8, int windowSize=1024, int rate=44100);
		std::vector<double> synthesize();

		int length();

		void transpose(double inFreq,double outFreq);
		void transpose(std::vector<double>);
		void amplify(double factor);
		void setLength(double start, double end, double nuvolength);

		double getCentroid(int hopstart, int hopend);
		void setCentroid(double centroid, int hopstart, int hopend);
		void printFreqs(int hopstart, int hopend);
};
