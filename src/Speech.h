#include <vector>
#include <iostream>
#include <functional>

#include "Sound.h"

class Speech{
	/* This class contains audio information,
	** but instead of storing every frequency,
	** it only stores the harmonics and how much power is in each of them.
	** It is therefore only suited to holding tonal sounds.
	*/
	private:
		//timing information
		int windowLength; // samples per fourier transform window function
		int hop; // samples between the start of each frame
		int hops; // total number of frames
		
		/* time-based data:
		**	Each vector contains an element for each frame.
		*/
		std::vector< std::vector<float> > magnitudes; //magnitudes for each harmonic in each frame
		// percent difference from the actual frequency
		// It's questionable if this is really useful.
		std::vector< std::vector<float> > freqDisplacements;
		std::vector<float> baseFrequencies; // the fundamental frequency of the formant curves
		std::vector<float> frequencies; // the fundamental frequency of each frame

		//helper methods/data
		std::vector<float> remainder; // the part of one synthesis that leaks into the next frames' times
		Sound toSound(int endHop); // converts the first endHop frames into a Sound object.

		bool verify();
	public:
		//public knowledge
		float sampleRate; // in Hz TODO:make all  sample rates floats
		float duration; //total length in seconds

		//creation, addition, & destruction
		Speech(Sound sample=Sound(),float frequency=20000.); //creates a speech from a monotonous Sound object
		void add(Speech addee, float overlap);// adds addee to the end, with overlap seconds of overlap
		Sound startToSound(float endTime);// returns the first endTime seconds as a Sound object
		void crop(float starttime, float endtime);// crops down to times between starttime and endtime
		std::vector<float> pop(float popLength);// removes up to popLength seconds from the start of the Speech
		                                        // and returns the removed part as audio
		std::vector<float> synthesize();// returns the whole Speech as audio

		//modifying
		/* stretches the section of the sound from start to end (in seconds).
		** The new length of that section will be nuvolength seconds.
		*/
		void stretch(float start, float end, float nuvolength);
		// Makes the sound louder by multiplying amplitudes at time t by gain(t)
		void amplify(std::function<float (float)> gain);
		// changes the fundamental frequency at time t to frequency(t)
		// It stops after endTime seconds.
		void transpose(std::function<float (float)> frequency, float endTime);

		//for reading & writing to disc
		Speech(std::istream& filestream);
		void write(std::ostream& filestream);
};
