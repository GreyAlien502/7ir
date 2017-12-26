#include <vector>
#include <complex>

int samplesFromTime(float time);
float timeFromSamples(int samples);

class Sound{
	private:
		void lengthenVector(std::vector<std::vector<float> >& input, int start, int end, int nuvolength);
	public:
		int windowLength;
		int hop;
		int hops;
		std::vector<float> rawSynthesize();

		std::vector< std::vector<float> > magnitudes;
		std::vector< std::vector<float> > frequencies;

		float duration;
		int sampleRate;

		Sound(std::vector<float> pcm=std::vector<float>(0,0), int overlap=8, int windowSize=1024, int rate=44100);
		Sound compatibleSound(std::vector<float> pcm);
		std::vector<float> synthesize();
};
