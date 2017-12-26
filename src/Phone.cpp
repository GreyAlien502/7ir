#include <vector>
#include <string>
#include <algorithm>

#include <iostream>

#include "fileio.h"
#include "Phone.h"

using namespace std;
/*
	float detectFrequency(vector<float> pcm,float sampleRate){
		int length = pcm.size();
		int minPeriod = floor(sampleRate/1046.5);//maximum frequency humans can sing
		int maxPeriod = floor(sampleRate/82.407);//minimum frequency humans can sing
		vector<float> errors(maxPeriod-minPeriod,0);
		for(int period=minPeriod; period<maxPeriod; period++){
			int periods = length/period;//number of periods that fit in the sample
			for(int periodsIn=0; periodsIn<periods-1; periodsIn++){
				for(int t=0;t<period;t++){
					errors[period-minPeriod] += pow( pcm[period*(periodsIn) + t] * pcm[period*(periodsIn+1) + t],2);
				}
			}
			errors[period-minPeriod] /= (periods-1)*period;
		}
		return float(sampleRate)/(minPeriod+distance( errors.begin(), max_element(errors.begin(),errors.end()) ));
	}
	float detectEnergy(vector<float> pcm){
		float energy =0.;
		for(int i=0; i<pcm.size(); i++){
			energy += pcm[i]*pcm[i];
		}
		return energy;
	}
	*/
Phone::Phone(
		vector<float> pcm,
		float consonantTime, float preutterTime, float overlapTime,
		int windowOverlap, int windowSize, int sampleRate){

	//initialize class variables
	consonant = consonantTime;
	preutter = preutterTime;
	overlap =  overlapTime;
	vector<float> vowelPart = vector<float>(
		pcm.begin()+consonant*sampleRate,
		pcm.end()
	);
	//float frequency = detectFrequency(vowelPart, sampleRate);
	 /*
	float powerroot = sqrt(detectEnergy(vowelPart))/pcm.size();
	cerr<<powerroot<<endl;
	for(int i=0; i<pcm.size(); i++){
		pcm[i] /= powerroot;
	}
	//*/
	if(overlap<0){
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
	if(overlap<0){
		overlap *= -1;
		sample = Speech(
			speechSample.startToSound(0).compatibleSound(vector<float>(overlap*speechSample.sampleRate))
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


Phone::Phone(istream& filestream){
	this->consonant = fileio::read(filestream,float(0));
	this->preutter  = fileio::read(filestream,float(0));
	this->overlap   = fileio::read(filestream,float(0));
	this->sample = Speech(filestream);
}
void Phone::write(ostream& filestream){
	fileio::write(filestream,this->consonant);
	fileio::write(filestream,this->preutter);
	fileio::write(filestream,this->overlap);
	sample.write(filestream);
}
