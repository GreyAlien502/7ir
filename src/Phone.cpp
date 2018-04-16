#include "fileio.h"
#include "Phone.h"

using namespace std;
Phone::Phone(
	vector<float> pcm,
	float consonantTime, float preutterTime, float overlapTime,
	int windowOverlap, int windowSize, int sampleRate
){
	//initialize class variables
	consonant = consonantTime;
	preutter = preutterTime;
	overlap =  overlapTime;
	vector<float> vowelPart = vector<float>(
		pcm.begin()+consonant*sampleRate,
		pcm.end()
	);
	if(overlap<0){ //negative overlap actually means to add silence to the beginning of the sound.
		overlap *= -1;
		pcm.insert(pcm.begin(),overlap*sampleRate,0);
		preutter += overlap;
	}
	sample = Speech(Sound(pcm, windowOverlap, windowSize, sampleRate));
}
Phone::Phone(Speech speechSample, float consonantTime, float preutterTime, float overlapTime){
	consonant = consonantTime;
	preutter = preutterTime;
	overlap = overlapTime;
	if(overlap<0){ //negative overlap actually means to add silence to the beginning of the sound.
		overlap *= -1;
		sample = Speech( // the right kind of silence
			speechSample.startToSound(0).compatibleSound(
				vector<float>(overlap*speechSample.sampleRate)
			)
		);
		sample.add(speechSample,0);
		preutter += overlap;
	} else {
		sample = speechSample;
	}
}
Phone::Phone(int overlapFactor, int windowLength, int sampleRate){
	consonant = preutter = overlap = 0;
	sample = Speech(Sound(
		vector<float>(windowLength),
		overlapFactor, windowLength, sampleRate
	));
}


float Phone::getConsonant(){ return consonant; }
float Phone::getPreutter (){ return preutter; }
float Phone::getOverlap  (){ return overlap; }

float Phone::getPreeffectLength(){ return preutter; }
