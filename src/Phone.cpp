#include <vector>
#include <string>
#include <algorithm>

#include <iostream>

#include "fileio.h"
#include "Phone.h"

using namespace std;

	double detectFrequency(vector<double> pcm,double sampleRate){
		int length = pcm.size();
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
Phone::Phone(
		vector<double> pcm,
		double consonantTime, double preutterTime, double overlapTime,
		int windowOverlap, int windowSize, int sampleRate){

	//initialize class variables
	consonant = consonantTime;
	preutter = preutterTime;
	overlap =  overlapTime;
	vector<double> vowelPart = vector<double>(
		pcm.begin()+consonant*sampleRate,
		pcm.end()
	);
	double frequency = detectFrequency(vowelPart, sampleRate);
	 /*
	double powerroot = sqrt(detectEnergy(vowelPart))/pcm.size();
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
	sample = Speech(Sound(pcm, windowOverlap, windowSize, sampleRate),frequency);
}
Phone::Phone(int overlapFactor, int windowLength, int sampleRate){
	consonant = preutter = overlap = 0;
	sample = Speech(Sound(
		vector<double>(windowLength),
		overlapFactor, windowLength, sampleRate
	), 440);
}


double Phone::getConsonant(){ return consonant; }
double Phone::getPreutter (){ return preutter; }
double Phone::getOverlap  (){ return overlap; }


Phone::Phone(istream& filestream){
	filestream.read(reinterpret_cast<char*>(&consonant),sizeof(consonant));
	filestream.read(reinterpret_cast<char*>(&preutter),sizeof(preutter));
	filestream.read(reinterpret_cast<char*>(&overlap),sizeof(overlap));
	sample = Speech(filestream);
}
void Phone::write(ostream& filestream){
	filestream.write(reinterpret_cast<char*>(&consonant),sizeof(consonant));
	filestream.write(reinterpret_cast<char*>(&preutter),sizeof(preutter));
	filestream.write(reinterpret_cast<char*>(&overlap),sizeof(overlap));
	sample.write(filestream);
}
