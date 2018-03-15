#include <vector>

int samplesFromTime(float time);
float timeFromSamples(int samples);

class Sound{
	/* This class stores audio data as overlapping frames
	** consisting of pairs of frequencies and magnitudes.
	*/
	private:
		/*makes vector input longer by stretching a part of it out.
		**	input: the vector to stretch
		**	start: the index of the beginning of the section to stretch
		**	end: the index of the end of the section to stretch
		**	nuvolength: the new length of the section after stretching
		*/
		void lengthenVector(std::vector<std::vector<float> >& input, int start, int end, int nuvolength);
		
		bool verify();
	public:
		int windowLength; // size of fourier transform window
		int hop; //samples between the start of consecutive frames
		int hops;// total number of frames
		std::vector<float> rawSynthesize(); //synthesize vector with fade in and fade out

		std::vector< std::vector<float> > magnitudes;// magnitudes of components 
		std::vector< std::vector<float> > frequencies;// corresponding frequencies of the components

		float duration; // length of the sound in seconds
		int sampleRate; // sampleRate in Hz

		//default to an empty sound with some values.
		Sound(std::vector<float> pcm=std::vector<float>(0,0), int overlap=8, int windowSize=1024, int rate=44100);
		// This turns pcm into a Sound with the same parameters as "this".
		Sound compatibleSound(std::vector<float> pcm);
		std::vector<float> synthesize(); // produces pcm audio of length duration from the samples.
};
