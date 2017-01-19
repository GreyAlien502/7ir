#include <vector>
#include <iostream>

#include "Voice.h"

using namespace std;

int Voice::getHop(double time){
	int hopnum = (sampleRate*time -windowLength/2)/hop;
	if(hopnum < 0){hopnum=0;}
	if(hopnum > hops){hopnum=hops;}
	return hopnum;
}

//make sound based off of input pcm data
Voice::Voice(Sound sample, double freq){
	//set class variables
	sampleRate = sample.sampleRate;
	windowLength = sample.windowLength;
	hop = sample.hop;
	hops = sample.hops;
	duration = sample.duration;

	magnitudes = vector<vector<double>>(hops,vector<double>(sampleRate/freq+1));
	freqDisplacements = vector<vector<double>>(hops,vector<double>(sampleRate/freq+1));
	frequencies = vector<double>(hops,0);

	//detect peaks
	for(int hopnum=0; hopnum<hops; hopnum++){
		frequencies[hopnum] = freq;
		vector<double> harmonicIndices = vector<double>(sampleRate/freq+1);
		for(int scannedIndex=0; scannedIndex<windowLength/2+1; scannedIndex++){
			int harmonic = sample.frequencies[hopnum][scannedIndex]/freq+.5;

			if( 1 > harmonic || harmonic > sampleRate/freq+1 ){ continue; }
			//if(harmonicIndices[harmonic] == 0){
			//	harmonicIndices[harmonic] = scannedIndex;
			//	continue;
			//}

			if( sample.magnitudes[hopnum][harmonicIndices[harmonic]] < sample.magnitudes[hopnum][scannedIndex]){
				harmonicIndices[harmonic] = scannedIndex;
			}
		}
		for(int harmonic=1; harmonic<harmonicIndices.size(); harmonic++){
			if(harmonicIndices[harmonic] == 0){
				magnitudes[hopnum][harmonic]=0;
				freqDisplacements[hopnum][harmonic]=0;
			}else{
				magnitudes[hopnum][harmonic] =
					sample.magnitudes[hopnum][harmonicIndices[harmonic]];
				freqDisplacements[hopnum][harmonic] =
					sample.frequencies[hopnum][harmonicIndices[harmonic]]/harmonic/freq-1;
			}
		}
	}
}
//makes a pcm vector from the sound
vector<double> Voice::synthesize(){
	Sound sample = Sound(vector<double>(duration*sampleRate,0),windowLength/hop,windowLength,sampleRate);
	cerr<<getHop(duration)<<'.'<<sample.magnitudes.size()<<'.'<<sample.frequencies.size();
	for(int hopnum=0; hopnum<hops; hopnum++){
		//add new frequencies
		for(int nuvoharmonic=1; nuvoharmonic<magnitudes[0].size(); nuvoharmonic++){
			double mag = magnitudes[hopnum][nuvoharmonic];
			double freq = (1+freqDisplacements[hopnum][nuvoharmonic])*nuvoharmonic*frequencies[hopnum];

			int i = freq/sampleRate*windowLength;
			if(0 < i&&i < windowLength/2+1){
				sample.frequencies[hopnum][i] = freq;
				sample.magnitudes[hopnum][i] = mag;
			}
		}
	}
	return sample.synthesize();
}

void Voice::transpose(double targetFreq){
	for(int hopnum=0; hopnum<hops; hopnum++){
		//interpolate
		double initFreq = frequencies[hopnum];
		vector<double> nuvomagnitudes = vector<double>(magnitudes[0].size(),0);
		vector<double> nuvofreqDisplacements = vector<double>(magnitudes[0].size(),0);
		for(int newHarmonic=1; newHarmonic < magnitudes[0].size()-1; newHarmonic++){
			int oldHarmonic = newHarmonic * targetFreq/initFreq;
			if(oldHarmonic>=magnitudes[0].size()-1){continue;}
			double interpolationFactor = newHarmonic/initFreq - oldHarmonic/targetFreq;
			nuvomagnitudes[newHarmonic] =
				 magnitudes[hopnum][oldHarmonic]*(1-interpolationFactor)
				+magnitudes[hopnum][oldHarmonic+1]*(interpolationFactor);
			nuvofreqDisplacements[newHarmonic] =
				 freqDisplacements[hopnum][oldHarmonic]*(1-interpolationFactor)
				+freqDisplacements[hopnum][oldHarmonic+1]*(interpolationFactor);
		}
		magnitudes[hopnum] = nuvomagnitudes;
		freqDisplacements[hopnum] = nuvofreqDisplacements;
	}
}

void Voice::amplify(double factor){
	for(int hopnum=0; hopnum<hops; hopnum++){
		for(int freq=0; freq<magnitudes[0].size(); freq++){
			magnitudes[hopnum][freq] *= factor;
		}
	}
}


	/*lengthens subsection vector of vector of doubles
	**adds extra elements or removes elements from start to end indices
	**to make the distance between them nuvolength.
	*/
template<typename contents>
void lengthenVector(vector<contents> & input, int start, int end, int nuvolength){
	int veclength = end-start;
	if(nuvolength<veclength){
		input.erase(input.begin()+start+nuvolength, input.begin()+end);
	}else{
		input.insert(input.begin()+start+veclength, nuvolength - veclength, input[0]);
		for(int i=nuvolength-1; i>0; i--){
			input[start+i] = input[start+i*veclength/nuvolength];
		}
	}
}
template void lengthenVector(vector<double>&,int,int,int);
template void lengthenVector(vector<vector<double>>&,int,int,int);

	/*expands or contracts the region of the sound between
	**times start and end to be size nuvolength
	**adding similar sound in the new region
	**if a new regian must be created.
	*/
void Voice::stretch(double start, double end, double nuvolength){
	int startHop = getHop(start);
	int   endHop =   getHop(end);
	int nuvohops;
	int soundSize = (duration+nuvolength-(end-start))*sampleRate;
	if(soundSize<windowLength){
		nuvohops = 1;
	}else{
		nuvohops = (soundSize - windowLength +hop*2-1)/hop - hops + (endHop-startHop);
	}
	cerr<<nuvohops<<endl;

	lengthenVector(magnitudes,  startHop, endHop, nuvohops);
	lengthenVector(freqDisplacements, startHop, endHop, nuvohops);
	lengthenVector(frequencies, startHop, endHop, nuvohops);
	
	hops += nuvohops-(endHop-startHop);
	duration += nuvolength-(end-start);
}
