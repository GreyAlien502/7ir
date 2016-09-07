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



Phone::Phone(vector<double> pcm,
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
	cerr<<frequency;
	 /*
	double powerroot = sqrt(detectEnergy(vowelPart))/pcm.size();
	cerr<<powerroot<<endl;
	for(int i=0; i<pcm.size(); i++){
		pcm[i] /= powerroot;
	}
	//*/
	sample = Sound(pcm,windowOverlap, windowSize, sampleRate);
}

Phone Phone::adjustPhone(Note& note){
	Phone output = *this;
	output.sample.printFreqs(
		output.consonant,
		output.sample.hops
	);
	double centroid = output.sample.getCentroid(
		output.consonant,
		output.sample.hops
	);
	output.sample.transpose( 440.*pow(2.,(note.notenum-69.)/12.) / frequency );
	output.sample.setCentroid(
		centroid*2,
		output.consonant,
		output.sample.hops
	);
	cerr<< ',';output.sample.getCentroid(
		output.consonant,
		output.sample.hops
	);
	output.sample.setLength(
		output.consonant,
		output.sample.hops,
		note.duration*sample.sampleRate/sample.hop
	);
	output.sample.amplify(note.velocity*1);
	return output;
}

double Phone::getConsonant(){ return sample.hop*consonant; }
double Phone::getPreutter(){ return sample.hop*preutter; }
double Phone::getOverlap(){ return sample.hop*overlap; }
