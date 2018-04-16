#include <vector>
#include <iostream>

#include "Speech.h"
#include "Note.h"

class Phone{
	/* This class contains an individual sound.
	*/
	protected:
	public:
		float consonant; //all in seconds
		float preutter; // all same as in initializer (below)
		float overlap;
		Speech sample;

		// initialize from:
		//	pcm: input sound as raw data
		//
		//	consonantTime: how long before consonant ends and vowel sound begins?
		//	preutterTime: how long before the vowel sound begins?
		//	overlapTime: how much should the previous sound overlap the beginning of this one?
		//
		//	windowOverlap: how much overlap between fourier transform windows?
		//	windowSize: how many samples should each fourier transform window be?
		//	sampleRate: audio sample rate
		Phone(
			std::vector<float> pcm,
			float consonantTime, float preutterTime, float overlapTime,
			int windowOverlap, int windowSize, int sampleRate
		);
		// initialize from:
		//	soundSample: input sound as Sound object
		//
		//	consonantTime, preutterTime, overlapTime: as above
		Phone(
			Speech soundSample,
			float consonantTime, float preutterTime, float overlapTime
		);

		// initialize empty phone with 0 duration using:
		//	windowOverlap, windowSize, sampleRate: as above
		Phone( int windowOverlap, int windowSize, int sampleRate);

		float getConsonant();
		float getPreutter();
		float getOverlap();
		
		/* get the amount of time before the note begins
		** that this phone will begin to effect the sound of the previous phone.
		*/
		float getPreeffectLength();
};
