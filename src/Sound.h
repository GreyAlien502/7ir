#include <vector>
#include <complex>

int samplesFromTime(double time);
double timeFromSamples(int samples);

class Sound{
	private:
		void lengthenVector(std::vector<std::vector<double> >& input, int start, int end, int nuvolength);
		std::vector< std::vector<double> > magnitudes;
		std::vector< std::vector<double> > frequencies;
	public:
		int sampleRate;
		int windowLength;
		int overlap;
		int hop;
		int hops;

		Sound(std::vector<double> pcm=std::vector<double>(2048,0), int overlap=16, int windowSize=2048, int rate=44100);
		std::vector<double> synthesize();

		int length();

		void transpose(double);
		void transpose(std::vector<double>);
		void amplify(double factor);
		void setLength(int start, int end, int nuvohopnum);

		double getCentroid(int hopnum);
		void setCentroid(int hopnum, double centroid);
};
