#include <vector>
#include <algorithm>
#include <iostream>
#include <functional>
#include <stdlib.h>
#include <math.h>

#include "Speech.h"

using namespace std;


//makes a Sound object from the Speech
Sound Speech::toSound(int endHop){
	Sound sample = Sound(vector<double>((endHop-1)*hop+windowLength,0),windowLength/hop,windowLength,sampleRate);
	for(int hopnum=0; hopnum<endHop; hopnum++){
		//add new frequencies
		for(int nuvoharmonic=1; nuvoharmonic<magnitudes[hopnum].size(); nuvoharmonic++){
			double mag = magnitudes[hopnum][nuvoharmonic];
			double freq = (1+freqDisplacements[hopnum][nuvoharmonic])*nuvoharmonic*frequencies[hopnum];

			int i = freq/sampleRate*windowLength+.5;
			if((0 < i) && (i < windowLength/2+1)){
				sample.frequencies[hopnum][i] = freq;
				sample.magnitudes[hopnum][i] = mag;
			}
		}
	}
	return sample;
}
Sound Speech::startToSound(double endTime){
	return toSound(endTime/sampleRate/hop);
}

void Speech::crop(double startTime, double endTime){
	double nuvoduration = endTime - startTime;
	int nuvohops = int(nuvoduration*sampleRate)/hop +1;
	int startHop = startTime*sampleRate/hop;
	int endHop = startHop + nuvohops;

	frequencies = vector<double>(frequencies.begin()+startHop,frequencies.begin()+endHop);
	magnitudes = vector<vector<double>>(magnitudes.begin()+startHop,magnitudes.begin()+endHop);
	freqDisplacements = vector<vector<double>>(freqDisplacements.begin()+startHop,freqDisplacements.begin()+endHop);

	duration = nuvoduration;
	hops = nuvohops;
}

double Speech::detectFrequency(vector<double> amplitudes,vector<double> frequencies){
	double minFreq = 82.407;//minimum frequency humans can sing
	int maxAmplitude = distance(
			amplitudes.begin(),
			max_element(amplitudes.begin()+minFreq*windowLength/sampleRate,amplitudes.end())
		);
	double maxFreq = frequencies[maxAmplitude];
	if(maxFreq<0){maxFreq=0;}
	int maxFreqNum =  maxFreq/minFreq;
	vector<double> correlations(maxFreqNum,1);
	for(int freqNum = 1; freqNum < maxFreqNum; freqNum +=1){
		double currentFreq = maxFreq/freqNum;
		for(int harmonic=0;harmonic<amplitudes.size()/maxFreq*sampleRate/windowLength;harmonic++){
			int currentIndex = currentFreq*harmonic/sampleRate*windowLength;
			correlations[freqNum-1] += amplitudes[currentIndex];
		}
	}
	double ferq = distance(
		correlations.begin(),
		max_element(correlations.begin(),correlations.end())
	)+1;
	if(maxFreq/ferq<250|maxFreq/ferq>350){
	}
	//cout<<maxFreq<<','<<ferq<<','<<maxFreqNum<<','<<maxFreq/ferq<<endl;
	return maxFreq/ferq;
}

//make speech based on input Sound sample
Speech::Speech(Sound sample){
	//set class variables
	sampleRate = sample.sampleRate;
	windowLength = sample.windowLength;
	hop = sample.hop;
	hops = sample.hops;
	duration = sample.duration;
	remainder = vector<double>(windowLength-hop);

	frequencies = vector<double>(hops);
	magnitudes = vector<vector<double>>(hops);
	freqDisplacements = vector<vector<double>>(hops);

	//detect frequencies
	for(int hopnum=0; hopnum<hops; hopnum++){
		double freq = detectFrequency(sample.magnitudes[hopnum],sample.frequencies[hopnum]);

		frequencies[hopnum] = freq;
	}
	int midIndex = frequencies.size()/2;
	nth_element(
		frequencies.begin(),
		frequencies.begin()+midIndex,
		frequencies.end()
	);
	double freq = frequencies[midIndex];
	for(int hopnum=0; hopnum<hops; hopnum++){
		magnitudes[hopnum] = vector<double>(sampleRate/freq+1);
		freqDisplacements[hopnum] = vector<double>(sampleRate/freq+1);
	}
	//detect peaks
	for(int hopnum=0; hopnum<hops; hopnum++){
		frequencies[hopnum] = freq;
		vector<double> harmonicIndices = vector<double>(sampleRate/freq+1);
		for(int scannedIndex=0; scannedIndex<windowLength/2+1; scannedIndex++){
			int harmonic = sample.frequencies[hopnum][scannedIndex]/freq+.5;

			if( 1 > harmonic || harmonic > sampleRate/freq+1 ){ continue; }
			if(harmonicIndices[harmonic] == 0){
				harmonicIndices[harmonic] = scannedIndex;
				continue;
			}

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
void Speech::add(Speech addee, double overlap){
	double nuvoduration = duration + addee.duration - overlap;
	int nuvohops = int(nuvoduration*sampleRate)/hop+1;
	int overlapHops = (hops-1) + (addee.hops-1) - (nuvohops-1) +1;
	if(overlap>duration){
		cerr<<overlapHops<<"WaARNENIGK	"<<hops<<endl;
		cerr<<overlap<<"    WaARNENIGK	"<<duration<<endl;
	}
	if(
			( windowLength != addee.windowLength )
			|( hop != addee.hop )
			|( sampleRate != addee.sampleRate )
	  ){
		throw invalid_argument("incompatible speech samples");
	}
	if(overlapHops<0){cerr<<"UMM\n";}

	frequencies      .resize(nuvohops);
	magnitudes       .resize(nuvohops);
	freqDisplacements.resize(nuvohops);

	for(int hopnum=0;hopnum+1<overlapHops;hopnum++){
		double fadeFactor = double(hopnum)/overlapHops;
		int actualhop = hops-overlapHops+hopnum;

		frequencies[actualhop] *= (1-fadeFactor);
		frequencies[actualhop] += addee.frequencies[hopnum]*fadeFactor;

		magnitudes[actualhop].resize(addee.magnitudes[hopnum].size());
		freqDisplacements[actualhop].resize(addee.magnitudes[hopnum].size());
		for(int i=0;i<addee.magnitudes[hopnum].size();i++){
			magnitudes[actualhop][i] *= (1-fadeFactor);
			magnitudes[actualhop][i] = addee.magnitudes[hopnum][i]*fadeFactor;
		}
		for(int i=0;i<freqDisplacements[actualhop].size();i++){
			freqDisplacements[actualhop][i] *= (1-fadeFactor);
			freqDisplacements[actualhop][i] += addee.freqDisplacements[hopnum][i]*fadeFactor;
		}
	}

	copy(
			addee.frequencies.begin()+overlapHops,
			addee.frequencies.end(),
			frequencies.begin()+hops
	    );
	copy(
			addee.magnitudes.begin()+overlapHops,
			addee.magnitudes.end(),
			magnitudes.begin()+hops
	    );
	copy(
			addee.freqDisplacements.begin()+overlapHops,
			addee.freqDisplacements.end(),
			freqDisplacements.begin()+hops
	    );
	duration = nuvoduration;
	hops = nuvohops;
}
vector<double> Speech::pop(double requestedLength){
	cerr<<'S';
	int poppedHops = int(requestedLength*sampleRate)/hop;//the number of hops that will be synthesized
	int nuvohops = hops - poppedHops;
	double nuvoduration = duration - poppedHops*hop/double(sampleRate);

	Sound outSound = toSound(poppedHops+1);
	cerr<<'E';
	vector<double> pcm = outSound.rawSynthesize();
	cerr<<'R';
	for(int i=0;i<remainder.size();i++){
		pcm[windowLength/2+i] += remainder[i];
	}
	cerr<<'E';

	frequencies.erase(frequencies.begin(),frequencies.begin()+poppedHops);
	magnitudes.erase(magnitudes.begin(),magnitudes.begin()+poppedHops);
	freqDisplacements.erase(freqDisplacements.begin(),freqDisplacements.begin()+poppedHops);


	hops = nuvohops;
	duration = nuvoduration;

	cerr<<'E';
	remainder = vector<double>(pcm.end() - (windowLength+1)/2,pcm.end());
	return vector<double>(pcm.begin()+windowLength/2,pcm.begin()+windowLength/2+poppedHops*hop);
}
vector<double> Speech::synthesize(){
	return toSound(hops).synthesize();
}


		/*lengthens subsection vector of vector of doubles
		 **adds extra elements or removes elements from start to end indices
		 **to make the distance between them nuvolength.
		 */
	template<typename contents>
	void lengthenVector(vector<contents> & input, int start, int length, int nuvolength){
		if(nuvolength<length){
			input.erase(input.begin()+start+nuvolength, input.begin()+start+length);
		}else{
			input.insert(input.begin()+start, nuvolength - length, input[0]);
			for(int i=0; i<nuvolength; i++){
				input[start+i] = input[start+nuvolength-length+i*length/nuvolength];
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
void Speech::stretch(double start, double end, double nuvolength){
	double length = end - start;
	double nuvoduration = duration + nuvolength - length;
	int nuvohops = nuvoduration*sampleRate/hop +1;
	int startHop = start*sampleRate/hop;
	int lengthHop = length*sampleRate/hop;
	int nuvolengthHop = (nuvohops-1) - (hops-1) + lengthHop;

	lengthenVector(magnitudes,       startHop, lengthHop, nuvolengthHop);
	lengthenVector(freqDisplacements,startHop, lengthHop, nuvolengthHop);
	lengthenVector(frequencies,      startHop, lengthHop, nuvolengthHop);

	hops = nuvohops;
	duration = nuvoduration;
}
void Speech::transpose(function<double(double)>nuvofreq, double endTime){
	int endHop = endTime*sampleRate/hop;
	if(frequencies.size()<endHop){
		cerr<<"bad sized boy already";
		cerr<<frequencies.size()<<'	'<<magnitudes.size()<<'	'<<endHop;
		cerr<<endl<<endTime<<endl<<duration<<endl;
	}
	for(int hopnum=0; hopnum<endHop; hopnum++){
		//interpolate
		double initFreq = frequencies[hopnum];
		double targetFreq = nuvofreq(hopnum*hop/sampleRate);
		vector<double> nuvomagnitudes = vector<double>(magnitudes[hopnum].size(),0);
		vector<double> nuvofreqDisplacements = vector<double>(magnitudes[hopnum].size(),0);
		for(int newHarmonic=1; newHarmonic+1 < magnitudes[hopnum].size(); newHarmonic++){
			int oldHarmonic = newHarmonic * targetFreq/initFreq;
			if(oldHarmonic+1>=magnitudes[hopnum].size()){continue;}
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
		frequencies[hopnum] = targetFreq;
	}
}
void Speech::amplify(function<double(double)>factors){
	for(int hopnum=0; hopnum<hops; hopnum++){
		double factor = factors( hopnum*hop/sampleRate);
		for(int freq=0; freq<magnitudes[0].size(); freq++){
			magnitudes[hopnum][freq] *= factor;
		}
	}
}


void Speech::write(ostream& filestream){
	filestream.write(reinterpret_cast<char*>(&windowLength),sizeof(windowLength));
	filestream.write(reinterpret_cast<char*>(&hop),sizeof(hop));
	filestream.write(reinterpret_cast<char*>(&hops),sizeof(hops));
	filestream.write(reinterpret_cast<char*>(&sampleRate),sizeof(sampleRate));
	filestream.write(reinterpret_cast<char*>(&duration),sizeof(duration));

	filestream.write(reinterpret_cast<char*>(&frequencies[0]),hops*sizeof(frequencies[0]));

	int magSize, freqDispSize;
	for(int hopnum=0; hopnum<hops; hopnum++){
		magSize = magnitudes[hopnum].size();
		filestream.write(
			reinterpret_cast<char*>(&magSize),
			sizeof(magSize));
		filestream.write(
			reinterpret_cast<char*>(&magnitudes[hopnum][0]),
			magnitudes[hopnum].size()*sizeof(magnitudes[hopnum][0]));

		freqDispSize = freqDisplacements[hopnum].size();
		filestream.write(
			reinterpret_cast<char*>(&freqDispSize),
			sizeof(freqDispSize));
		filestream.write(
			reinterpret_cast<char*>(&freqDisplacements[hopnum][0]),
			freqDisplacements[hopnum].size()*sizeof(freqDisplacements[hopnum][0]));
	}
}
Speech::Speech(istream& filestream){
	filestream.read(reinterpret_cast<char*>(&windowLength),sizeof(windowLength));
	filestream.read(reinterpret_cast<char*>(&hop),sizeof(hop));
	filestream.read(reinterpret_cast<char*>(&hops),sizeof(hops));
	filestream.read(reinterpret_cast<char*>(&sampleRate),sizeof(sampleRate));
	filestream.read(reinterpret_cast<char*>(&duration),sizeof(duration));

	frequencies = vector<double>(hops);
	magnitudes = vector<vector<double>>(hops);
	freqDisplacements = vector<vector<double>>(hops);
	filestream.read(reinterpret_cast<char*>(&frequencies[0]),hops*sizeof(frequencies[0]));

	int magSize, freqDispSize;
	for(int hopnum=0; hopnum<hops; hopnum++){
		filestream.read(reinterpret_cast<char*>(&magSize),sizeof(magSize));
		magnitudes[hopnum] = vector<double>(magSize);
		filestream.read(
			reinterpret_cast<char*>(&magnitudes[hopnum][0]),
			magSize*sizeof(magnitudes[hopnum][0]));

		filestream.read(reinterpret_cast<char*>(&freqDispSize),sizeof(freqDispSize));
		freqDisplacements[hopnum] = vector<double>(freqDispSize);
		filestream.read(
			reinterpret_cast<char*>(&freqDisplacements[hopnum][0]),
			freqDispSize*sizeof(freqDisplacements[hopnum][0]));
	}
}






/*returns the value of vec at index
**returns 0 if that element doesn't exist
double tryAt(vector<double>& vec, int index){
	if(index < vec.size()){
		return vec[index];
	}else{
		return 0;
	}
}
*/
