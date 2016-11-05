#include <vector>
#include <string>
#include <algorithm>

#include <iostream>

#include "fileio.h"
#include "VoiceLibrary.h"

using namespace std;

double detectFrequency(vector<double> pcm,double sampleRate){
	unsigned int length = pcm.size();
	int minPeriod = floor(sampleRate/1046.5);//maximum frequency humans can sing
	int maxPeriod = floor(sampleRate/82.407);//minimum frequency humans can sing
	vector<double> errors(maxPeriod-minPeriod,0);
	for(int period=minPeriod; period<maxPeriod; period++){
		int periods = length/period;//number of periods that fit in the sample
		for(int periodsIn=0; periodsIn<periods-1; periodsIn++){
			for(int t=0;t<period;t++){
				errors[period-minPeriod] += pow( pcm[period*(periodsIn) + t] * pcm[period*(periodsIn+1) + t],2);
			}
		}
		errors[period-minPeriod] /= (periods-1)*period;
	}
	return double(sampleRate)/(minPeriod+distance( errors.begin(), max_element(errors.begin(),errors.end()) ));
}

double detectEnergy(vector<double> pcm){
	double energy =0.;
	for(int i=0; i<pcm.size(); i++){
		energy += pcm[i]*pcm[i];
	}
	return energy;
}



Phone::Phone(int cons, int preut, int overLap, Sound samp){
	consonant = cons;
	preutter = preut;
	overlap = overLap;
	sample = samp;
}
double Phone::getConsonant(){ return sample.hop*consonant; }
double Phone::getPreutter(){ return sample.hop*preutter; }
double Phone::getOverlap(){ return sample.hop*overlap; }



basePhone::basePhone(vector<double> pcm,
		double consonantTime, double preutterTime, double overlapTime,
		int windowOverlap, int windowSize, int sampleRate){

	int hop = windowSize/windowOverlap;

	//initialize class variables
	consonant = floor(consonantTime*sampleRate/hop);
	preutter = floor(preutterTime*sampleRate/hop);
	overlap = floor(overlapTime*sampleRate/hop);
	vector<double> vowelPart = vector<double>(
		pcm.begin()+consonant*hop,
		pcm.end()
	);
	frequency = detectFrequency(vowelPart, sampleRate);
	 /*
	double powerroot = sqrt(detectEnergy(vowelPart))/pcm.size();
	cerr<<powerroot<<endl;
	for(int i=0; i<pcm.size(); i++){
		pcm[i] /= powerroot;
	}
	//*/
	sample = Sound(pcm,windowOverlap, windowSize, sampleRate);
}

Phone basePhone::adjustPhone(Note& note, double tempo){
	Sound samp = sample;
	samp.transpose( 440.*pow(2.,(note.notenum-69.)/12.) / frequency );
	samp.setLength(
		consonant,
		samp.hops,
		note.duration /tempo  *samp.sampleRate/samp.hop
	);
	samp.amplify(note.velocity);
	return Phone(consonant, preutter, overlap, samp);
}

