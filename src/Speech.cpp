#include <algorithm> // max_element
#include <functional>// function<float (float)>
#include <math.h> // floor
#include <assert.h>

#include "Speech.h"
#include "fileio.h"

using namespace std;
/*
** resamplet thte input vector by a factor specified by the scaleFactor
** larger scaleFactor means more samples in the new vector compared to the old one.
*/
vector<float> resample(const vector<float>& input,float scaleFactor){
	vector<float> output = vector<float>(input.size()*scaleFactor);
	//assert(output.size()!=0);
	for(int outIndex=1; outIndex+1 < output.size(); outIndex++){
		int inIndex = outIndex * scaleFactor;//the index to draw from
		if(inIndex+1>=input.size()){continue;}//if you are past the end, leave it at zero.
		float interpolationFactor = outIndex*scaleFactor - inIndex;
		output[outIndex] =
			 input[inIndex]*(1-interpolationFactor)
			+input[inIndex+1]*(interpolationFactor);
	}
	return output;
}


bool Speech::verify(){
	assert( this->hops > 0);
	assert( this->duration >= 0);
	assert( this->hops == int(this->duration*sampleRate)/hop + 1);
	// magnitudes and frequencies should be <hops> long.
	assert(this->magnitudes.size()==this->hops);
	assert(this->baseFrequencies.size()==this->hops);
	assert(this->frequencies.size()==this->hops);
	for(int hopnum=0;hopnum<this->magnitudes.size();hopnum++){
		assert(
			this->magnitudes[hopnum].size()
			==
			this->freqDisplacements[hopnum].size()
		);
	}
	return true;
}
//makes a Sound object from the Speech
Sound Speech::toSound(int endHop){
	assert(endHop<=this->hops);
	Sound sample = Sound(
		vector<float>((endHop-1)*hop+windowLength,0),
		windowLength/hop,windowLength,sampleRate
	);//start with emtpy sound of proper length
	for(int hopnum=0; hopnum<endHop; hopnum++){
		float rescaleFactor = frequencies[hopnum] / baseFrequencies[hopnum];
		vector<float> outputMagnitudes = resample(magnitudes[hopnum],rescaleFactor);
		vector<float> outputFreqDisplacements = resample(freqDisplacements[hopnum],rescaleFactor);
		//add new frequencies
		for(int nuvoharmonic=1; nuvoharmonic<outputMagnitudes.size(); nuvoharmonic++){
			float mag = outputMagnitudes[nuvoharmonic];
			float freq = (1+outputFreqDisplacements[nuvoharmonic])
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
	int endHop = endTime/sampleRate/hop;
	if(endHop > this->hops){
		endHop = this->hops - 1;
	}
	return toSound(endHop);
}

void Speech::crop(float startTime, float endTime){
	float nuvoduration = endTime - startTime;
	int nuvohops = int(nuvoduration*sampleRate)/hop +1;
	int startHop = startTime*sampleRate/hop;
	int endHop = startHop + nuvohops;

	baseFrequencies = vector<float>(baseFrequencies.begin()+startHop,baseFrequencies.begin()+endHop);
	frequencies = vector<float>(frequencies.begin()+startHop,frequencies.begin()+endHop);
	magnitudes = vector<vector<float>>(magnitudes.begin()+startHop,magnitudes.begin()+endHop);
	freqDisplacements = vector<vector<float>>(freqDisplacements.begin()+startHop,freqDisplacements.begin()+endHop);

	duration = nuvoduration;
	hops = nuvohops;
	this->verify();
}


//make speech based on input Sound sample
Speech::Speech(Sound sample,float freq){
	const float MAXFREQ = 20000;
	//set class variables
	this->sampleRate = sample.sampleRate;
	this->windowLength = sample.windowLength;
	this->hop = sample.hop;
	this->hops = sample.hops;
	this->duration = sample.duration;
	this->remainder = vector<float>(windowLength-hop);

	this->baseFrequencies = vector<float>(hops);
	this->frequencies = vector<float>(hops);
	this->magnitudes = vector<vector<float>>(hops);
	this->freqDisplacements = vector<vector<float>>(hops);

	//detect frequencies
	for(int hopnum=0; hopnum<hops; hopnum++){
		baseFrequencies[hopnum] = freq;//TODO:remove this block; it's already in the other one
		frequencies[hopnum] = freq;
	}
	for(int hopnum=0; hopnum<hops; hopnum++){
		magnitudes[hopnum] = vector<float>(sampleRate/freq+1);
		freqDisplacements[hopnum] = vector<float>(sampleRate/freq+1);
	}
	//detect peaks
	for(int hopnum=0; hopnum<hops; hopnum++){
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
	this->verify();
}
void Speech::add(Speech addee, float overlap){
	if(overlap > addee.duration){ // stretch if your'e trying to overlap longer thtan the thing your addinge.
		addee.stretch(0,addee.duration,overlap);
	}
	if(overlap > this->duration){ // stretch if your'e trying to overlap longer thtan the thing your addinge.
		this->stretch(0,this->duration,overlap);
	}
	assert(overlap <= addee.duration);
	assert(overlap <= this->duration);

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
	baseFrequencies  .resize(nuvohops);
	frequencies      .resize(nuvohops);
	magnitudes       .resize(nuvohops);
	freqDisplacements.resize(nuvohops);

	//add overlap to the old sound by fading
	for(int hopnum=0;hopnum+1<overlapHops;hopnum++){
		int actualhop = hops-overlapHops+hopnum;//in frame of this

		float fadeFactor = float(hopnum)/overlapHops;
		float nuvobaseFreq =
			this->baseFrequencies[actualhop]*(1-fadeFactor) +
			addee.baseFrequencies[hopnum]*fadeFactor;
		float nuvofreq =
			this->frequencies[actualhop]*(1-fadeFactor) +
			addee.frequencies[hopnum]*fadeFactor;


		float sound1rescaleFactor = nuvobaseFreq / baseFrequencies[actualhop];
		vector<float> sound1magnitudes = resample( magnitudes[actualhop],sound1rescaleFactor );
		vector<float> sound1freqDisplacements = resample( freqDisplacements[actualhop],sound1rescaleFactor );

		float sound2rescaleFactor = nuvobaseFreq / addee.baseFrequencies[hopnum];
		vector<float> sound2magnitudes = resample( addee.magnitudes[hopnum], sound2rescaleFactor );
		vector<float> sound2freqDisplacements = resample( addee.freqDisplacements[hopnum], sound2rescaleFactor );

		sound1magnitudes.resize(sound2magnitudes.size());
		sound2magnitudes.resize(sound1magnitudes.size());
		sound1freqDisplacements.resize(sound2freqDisplacements.size());
		sound2freqDisplacements.resize(sound1freqDisplacements.size());

		this->magnitudes[actualhop]        = vector<float>(sound1magnitudes.size());
		this->freqDisplacements[actualhop] = vector<float>(sound1magnitudes.size());
		for(int i=0;i<sound1magnitudes.size();i++){
			float oldMag = sound1magnitudes[i];
			float addMag = sound2magnitudes[i];
			//float fadeFactor;
			//if(oldMag + addMag == 0){
			//	fadeFactor=0;
			//}else{
			//	fadeFactor =(addMag)/(oldMag + addMag);
			//}

			this->magnitudes[actualhop][i] =
				sound1magnitudes[i]*(1-fadeFactor) +
				sound2magnitudes[i]*fadeFactor;

			this->freqDisplacements[actualhop][i] =
				sound1freqDisplacements[i]*(1-fadeFactor) +
				sound2freqDisplacements[i]*fadeFactor;


			nuvofreq += frequencies[hopnum]*(1-fadeFactor);
			nuvofreq += addee.frequencies[hopnum]*fadeFactor;
		}
		this->baseFrequencies[actualhop] =
			this->baseFrequencies[actualhop]*(1-fadeFactor) +
			addee.baseFrequencies[hopnum]*fadeFactor;
		this->frequencies[actualhop] =
			this->frequencies[actualhop]*(1-fadeFactor) +
			addee.frequencies[hopnum]*fadeFactor;
	}

	//copy the new one in after the end of the old one
	copy(
		addee.baseFrequencies.begin()+overlapHops,
		addee.baseFrequencies.end(),
		baseFrequencies.begin()+hops
	);
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
	this->verify();
}
vector<float> Speech::pop(float requestedLength){
	int poppedHops = int(requestedLength*sampleRate)/hop;//the number of hops that will be synthesized
	if(poppedHops >= this->hops){
		cerr<<"This shouldn't happen too often";
		this->stretch(0,this->duration,requestedLength);
	}
	assert(poppedHops < this->hops);
	int nuvohops = hops - poppedHops;
	float nuvoduration = duration - poppedHops*hop/float(sampleRate);

	//write the first part in to a vector
	Sound outSound = this->toSound(poppedHops+1);
	vector<float> pcm = outSound.rawSynthesize();
	for(int i=0;i<remainder.size();i++){
		pcm[windowLength/2+i] += remainder[i];
	}

	//remove synthesized parts
	baseFrequencies.erase(baseFrequencies.begin(),baseFrequencies.begin()+poppedHops);
	frequencies.erase(frequencies.begin(),frequencies.begin()+poppedHops);
	magnitudes.erase(magnitudes.begin(),magnitudes.begin()+poppedHops);
	freqDisplacements.erase(freqDisplacements.begin(),freqDisplacements.begin()+poppedHops);


	hops = nuvohops;
	duration = nuvoduration;
	remainder = vector<float>(pcm.end() - (windowLength+1)/2,pcm.end());

	this->verify();
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
	static void lengthenVector(vector<contents> & input, int start, int length, int nuvolength){
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
	float nuvoduration = duration + (nuvolength - length);// these parenths minimize rounding errors (no error if nuvolength == length)
	int nuvohops = nuvoduration*sampleRate/hop +1;
	int startHop = start*sampleRate/hop;
	int lengthHop = (hops-1) - startHop;
	int nuvolengthHop = (nuvohops-1) - (hops-1) + lengthHop;
	if(nuvolengthHop < 0){
		cerr<<start<<'	'<<end<<'	'<<nuvolength<<"	3:	"
		<<nuvohops<<'	'<<hops<<'	'<<lengthHop;
	}

	lengthenVector(magnitudes,       startHop, lengthHop, nuvolengthHop);
	lengthenVector(freqDisplacements,startHop, lengthHop, nuvolengthHop);
	lengthenVector(baseFrequencies,  startHop, lengthHop, nuvolengthHop);
	lengthenVector(frequencies,      startHop, lengthHop, nuvolengthHop);

	hops = nuvohops;
	duration = nuvoduration;
	this->verify();
}
void Speech::transpose(function<float(float)>nuvofreq, float endTime){
	int endHop = endTime*sampleRate/hop;
	if(frequencies.size()<endHop){
		cerr<<"bad sized boy already";
		cerr<<frequencies.size()<<'	'<<magnitudes.size()<<'	'<<endHop;
		cerr<<endl<<endTime<<endl<<duration<<endl;
	}
	for(int hopnum=0; hopnum<endHop; hopnum++){
		// update frequency
		frequencies[hopnum] = nuvofreq(hopnum*hop/sampleRate);
	}
	this->verify();
}
void Speech::amplify(function<float(float)>factors){
	for(int hopnum=0; hopnum<hops; hopnum++){
		float factor = factors( hopnum*hop/sampleRate);
		for(int freq=0; freq<magnitudes[0].size(); freq++){
			magnitudes[hopnum][freq] *= factor;
		}
	}
	this->verify();
}


void Speech::write(ostream& filestream){
	fileio::write(filestream,this->windowLength);
	fileio::write(filestream,this->hop);
	fileio::write(filestream,this->hops);

	fileio::write(filestream,this->magnitudes);
	fileio::write(filestream,this->freqDisplacements);
	fileio::write(filestream,this->baseFrequencies);
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
	this->baseFrequencies       = fileio::read(filestream,vector<float>());
	this->frequencies       = fileio::read(filestream,vector<float>());

	this->remainder = fileio::read(filestream,vector<float>());

	this->duration   = fileio::read(filestream,float(0));
	this->sampleRate = fileio::read(filestream,float(0));

	this->verify();
}
