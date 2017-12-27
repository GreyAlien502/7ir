#include <algorithm> // max_element
#include <functional>// function<float (float)>
#include <math.h> // floor

#include "Speech.h"
#include "fileio.h"

using namespace std;


//makes a Sound object from the Speech
Sound Speech::toSound(int endHop){
	Sound sample = Sound(
		vector<float>((endHop-1)*hop+windowLength,0),
		windowLength/hop,windowLength,sampleRate
	);//start with emtpy sound of proper length
	for(int hopnum=0; hopnum<endHop; hopnum++){
		//add new frequencies
		for(int nuvoharmonic=1; nuvoharmonic<magnitudes[hopnum].size(); nuvoharmonic++){
			float mag = magnitudes[hopnum][nuvoharmonic];
			float freq = (1+freqDisplacements[hopnum][nuvoharmonic])
				*nuvoharmonic *frequencies[hopnum];

			int i = freq/sampleRate*windowLength+.5;
			if((0 < i) && (i < windowLength/2+1)){
				sample.frequencies[hopnum][i] = freq;
				sample.magnitudes[hopnum][i] = mag;
			}
		}
	}
	return sample;
}
Sound Speech::startToSound(float endTime){
	return toSound(endTime/sampleRate/hop);
}

void Speech::crop(float startTime, float endTime){
	float nuvoduration = endTime - startTime;
	int nuvohops = int(nuvoduration*sampleRate)/hop +1;
	int startHop = startTime*sampleRate/hop;
	int endHop = startHop + nuvohops;

	frequencies = vector<float>(frequencies.begin()+startHop,frequencies.begin()+endHop);
	magnitudes = vector<vector<float>>(magnitudes.begin()+startHop,magnitudes.begin()+endHop);
	freqDisplacements = vector<vector<float>>(freqDisplacements.begin()+startHop,freqDisplacements.begin()+endHop);

	duration = nuvoduration;
	hops = nuvohops;
}

//experimental frequency detection that hasn't been very accurate
/*float Speech::detectFrequency(vector<float> amplitudes,vector<float> frequencies){
	float minFreq = 82.407;//minimum frequency humans can sing
	int maxAmplitude = distance(
			amplitudes.begin(),
			max_element(amplitudes.begin()+minFreq*windowLength/sampleRate,amplitudes.end())
		);
	float maxFreq = frequencies[maxAmplitude];
	if(maxFreq<0){maxFreq=0;}
	int maxFreqNum =  maxFreq/minFreq;
	vector<float> correlations(maxFreqNum,1);
	for(int freqNum = 1; freqNum < maxFreqNum; freqNum +=1){
		float currentFreq = maxFreq/freqNum;
		for(int harmonic=0;harmonic<amplitudes.size()/maxFreq*sampleRate/windowLength;harmonic++){
			int currentIndex = currentFreq*harmonic/sampleRate*windowLength;
			correlations[freqNum-1] += amplitudes[currentIndex];
		}
	}
	float ferq = distance(
		correlations.begin(),
		max_element(correlations.begin(),correlations.end())
	)+1;
	if(maxFreq/ferq<250|maxFreq/ferq>350){
	}
	//cout<<maxFreq<<','<<ferq<<','<<maxFreqNum<<','<<maxFreq/ferq<<endl;
	return maxFreq/ferq;
}*/ //It's not very effective

// detects the fundamental frequency in an audio signal
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
	int maximum = distance( errors.begin(), max_element(errors.begin(),errors.end()) );

	//try to catch octave errors TODO:make (more?) effective
	int fundamentalIndex = maximum;
	/*
	for(int index=maximum/2; index*0>minPeriod; index--){
		if( errors[index] > errors[maximum]*.75 ){
			fundamentalIndex = (2*index+1)/(2*maximum);
		}
	}
	*/
	return float(sampleRate)/(minPeriod+fundamentalIndex);
}

//make speech based on input Sound sample
Speech::Speech(Sound sample){
	const float MAXFREQ = 20000;
	//set class variables
	this->sampleRate = sample.sampleRate;
	this->windowLength = sample.windowLength;
	this->hop = sample.hop;
	this->hops = sample.hops;
	this->duration = sample.duration;
	this->remainder = vector<float>(windowLength-hop);

	this->frequencies = vector<float>(hops);
	this->magnitudes = vector<vector<float>>(hops);
	this->freqDisplacements = vector<vector<float>>(hops);

	float freq = detectFrequency(sample.synthesize(),this->sampleRate);

	//detect frequencies
	for(int hopnum=0; hopnum<hops; hopnum++){
		frequencies[hopnum] = freq;
	}
	for(int hopnum=0; hopnum<hops; hopnum++){
		magnitudes[hopnum] = vector<float>(sampleRate/freq+1);
		freqDisplacements[hopnum] = vector<float>(sampleRate/freq+1);
	}
	//detect peaks
	for(int hopnum=0; hopnum<hops; hopnum++){
		frequencies[hopnum] = freq;
		vector<float> harmonicIndices = vector<float>(sampleRate/freq+1);
		//find the index of the largest signal in each harmonic range.
		for(int scannedIndex=0; scannedIndex<windowLength/2+1; scannedIndex++){
			// This is the harmonic that the frequency would count towards
			// if it were the strongest contribution to it.
			int harmonic = sample.frequencies[hopnum][scannedIndex]/freq+.5;

			// If harmonic is not reasonable, ignore it.
			if( 1 > harmonic || harmonic > sampleRate/freq+1 ){ continue; }
			// If harmonic has not been set already, set it.
			if(harmonicIndices[harmonic] == 0){
				harmonicIndices[harmonic] = scannedIndex;
				continue;
			}

			if( // If this is the biggest amplitude that would be at that harmonic,
				sample.magnitudes[hopnum][harmonicIndices[harmonic]]
				<
				sample.magnitudes[hopnum][scannedIndex]
			){// set it to the index of that harmonic.
				harmonicIndices[harmonic] = scannedIndex;
			}
		}
		// Now set the magnitudes and ferquency displacements based on the indices.
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
void Speech::add(Speech addee, float overlap){
	float nuvoduration = duration + addee.duration - overlap;
	int nuvohops = int(nuvoduration*sampleRate)/hop+1;
	int overlapHops = (hops-1) + (addee.hops-1) - (nuvohops-1) +1;
	if(overlap>duration){ //this should nont happen. if it does it's probably rounding error
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
	if(overlapHops<0){cerr<<"UMM\n";} // this should also not happen: for debugging only

	//extend the old ones to fit new ones
	frequencies      .resize(nuvohops);
	magnitudes       .resize(nuvohops);
	freqDisplacements.resize(nuvohops);

	//add overlap to the old sound by fading
	for(int hopnum=0;hopnum+1<overlapHops;hopnum++){
		float fadeFactor = float(hopnum)/overlapHops;
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

	//copy the new one in after the end of the old one
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
vector<float> Speech::pop(float requestedLength){
	int poppedHops = int(requestedLength*sampleRate)/hop;//the number of hops that will be synthesized
	int nuvohops = hops - poppedHops;
	float nuvoduration = duration - poppedHops*hop/float(sampleRate);

	//write the first part in to a vector
	Sound outSound = this->toSound(poppedHops+1);
	vector<float> pcm = outSound.rawSynthesize();
	for(int i=0;i<remainder.size();i++){
		pcm[windowLength/2+i] += remainder[i];
	}

	//remove synthesized parts
	frequencies.erase(frequencies.begin(),frequencies.begin()+poppedHops);
	magnitudes.erase(magnitudes.begin(),magnitudes.begin()+poppedHops);
	freqDisplacements.erase(freqDisplacements.begin(),freqDisplacements.begin()+poppedHops);


	hops = nuvohops;
	duration = nuvoduration;
	remainder = vector<float>(pcm.end() - (windowLength+1)/2,pcm.end());

	return vector<float>(pcm.begin()+windowLength/2,pcm.begin()+windowLength/2+poppedHops*hop);
}
vector<float> Speech::synthesize(){
	return toSound(hops).synthesize(); // synthesize everything
}


	/* lengthens subsection vector of vector of floats
	** adds extra elements or removes elements from start to end indices
	** to make the distance between them nuvolength.
	*/
	template<typename contents>
	void lengthenVector(vector<contents> & input, int start, int length, int nuvolength){
		if(nuvolength<length){
			// If it's being shortened, just delete the end.
			input.erase(input.begin()+start+nuvolength, input.begin()+start+length);
		}else{
			// Otherwise, add space and expand into it.
			input.insert(input.begin()+start, nuvolength - length, input[0]);
			for(int i=0; i<nuvolength; i++){
				input[start+i] = input[start+nuvolength-length+i*length/nuvolength];
			}
		}
	}
	template void lengthenVector(vector<float>&,int,int,int);
	template void lengthenVector(vector<vector<float>>&,int,int,int);
	/* expands or contracts the region of the sound between
	** times start and end to be size nuvolength
	** adding similar sound in the new region
	** if a new regian must be created.
	*/
void Speech::stretch(float start, float end, float nuvolength){
	float length = end - start;
	float nuvoduration = duration + nuvolength - length;
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
void Speech::transpose(function<float(float)>nuvofreq, float endTime){
	int endHop = endTime*sampleRate/hop;
	if(frequencies.size()<endHop){
		cerr<<"bad sized boy already";
		cerr<<frequencies.size()<<'	'<<magnitudes.size()<<'	'<<endHop;
		cerr<<endl<<endTime<<endl<<duration<<endl;
	}
	for(int hopnum=0; hopnum<endHop; hopnum++){
		//interpolate the new magnitudes
		//TODO:do this on synthesis
		float initFreq = frequencies[hopnum];
		float targetFreq = nuvofreq(hopnum*hop/sampleRate);
		vector<float> nuvomagnitudes = vector<float>(magnitudes[hopnum].size(),0);
		vector<float> nuvofreqDisplacements = vector<float>(magnitudes[hopnum].size(),0);
		for(int newHarmonic=1; newHarmonic+1 < magnitudes[hopnum].size(); newHarmonic++){
			int oldHarmonic = newHarmonic * targetFreq/initFreq;
			if(oldHarmonic+1>=magnitudes[hopnum].size()){continue;}
			float interpolationFactor = newHarmonic/initFreq - oldHarmonic/targetFreq;
			nuvomagnitudes[newHarmonic] =
				 magnitudes[hopnum][oldHarmonic]*(1-interpolationFactor)
				+magnitudes[hopnum][oldHarmonic+1]*(interpolationFactor);
			nuvofreqDisplacements[newHarmonic] =0;
				 freqDisplacements[hopnum][oldHarmonic]*(1-interpolationFactor)
				+freqDisplacements[hopnum][oldHarmonic+1]*(interpolationFactor);
		}
		magnitudes[hopnum] = nuvomagnitudes;
		freqDisplacements[hopnum] = nuvofreqDisplacements;
		frequencies[hopnum] = targetFreq;
	}
}
void Speech::amplify(function<float(float)>factors){
	for(int hopnum=0; hopnum<hops; hopnum++){
		float factor = factors( hopnum*hop/sampleRate);
		for(int freq=0; freq<magnitudes[0].size(); freq++){
			magnitudes[hopnum][freq] *= factor;
		}
	}
}


void Speech::write(ostream& filestream){
	fileio::write(filestream,this->windowLength);
	fileio::write(filestream,this->hop);
	fileio::write(filestream,this->hops);

	fileio::write(filestream,this->magnitudes);
	fileio::write(filestream,this->freqDisplacements);
	fileio::write(filestream,this->frequencies);

	fileio::write(filestream,this->remainder);

	fileio::write(filestream,this->duration);
	fileio::write(filestream,this->sampleRate);
}
Speech::Speech(istream& filestream){
	this->windowLength = fileio::read(filestream,int(0));
	this->hop          = fileio::read(filestream,int(0));
	this->hops         = fileio::read(filestream,int(0));

	this->magnitudes        = fileio::read(filestream,vector<vector<float>>());
	this->freqDisplacements = fileio::read(filestream,vector<vector<float>>());
	this->frequencies       = fileio::read(filestream,vector<float>());

	this->remainder = fileio::read(filestream,vector<float>());

	this->duration   = fileio::read(filestream,float(0));
	this->sampleRate = fileio::read(filestream,float(0));
}






/*returns the value of vec at index
**returns 0 if that element doesn't exist
float tryAt(vector<float>& vec, int index){
	if(index < vec.size()){
		return vec[index];
	}else{
		return 0;
	}
}
*/
