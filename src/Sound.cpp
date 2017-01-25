#include <vector>
#include <complex>
#include <cmath>
#include <iostream>
#include <stdexcept>

#include <fftw3.h>

#include "Sound.h"

using namespace std;

vector<double> hamming(int windowLength) {
	double PI = 3.14159265358979323;
	vector<double> output(windowLength);
	for(int i = 0; i < windowLength; i++) {
		output[i] = .5 - .5 * cos( 2 * PI * (i / (double) (windowLength) ));
	}
	return output;
}



int Sound::length(){
	return hops*hop+windowLength-hop;
}

int Sound::getHop(double time){
	int hopnum = (sampleRate*time -windowLength/2)/hop;
	if(hopnum < 0){hopnum=0;}
	if(hopnum > hops){hopnum=hops;}
	return hopnum;
}

//make sound based off of input pcm data
Sound::Sound(vector<double> pcm, int overlapFactor, int sizeOfWindow, int rate){
	if(sizeOfWindow%overlapFactor != 0){
		throw invalid_argument("Window size munt be a multiple of overlap factor.(");
	}

	double const PI = 3.14159265358979323846;
	vector<double> window = hamming(sizeOfWindow);

	//initialize object variables
	sampleRate = rate;
	windowLength = sizeOfWindow;
	overlap = overlapFactor;
	hop = windowLength/overlap;
	if(pcm.size()<sizeOfWindow){
		hops = 1;
	}else{
		hops = (pcm.size() - sizeOfWindow +hop*2-1)/hop;
	}

	duration = double(pcm.size())/rate;
	pcm.resize(windowLength + hops*hop-hop,0);

	magnitudes = vector<vector<double> > (hops, vector<double>(windowLength/2+1,0.));
	frequencies = vector<vector<double> > (hops, vector<double>(windowLength/2+1));


	//variables to use for fft
	double* in = (double*) fftw_malloc(sizeof(double)*(windowLength));
	complex<double>* out = (complex<double>*) fftw_alloc_complex(sizeof(fftw_complex)*(windowLength/2+1));
	fftw_plan toFreck = fftw_plan_dft_r2c_1d(
		windowLength,
		in,
		reinterpret_cast<fftw_complex*>(out),
		FFTW_MEASURE);


	vector<double> oldPhases = vector<double>(windowLength,0);//to calculate phase change
	int pos = 0;
	for(int hopnum=0; hopnum<hops; hopnum++){
		pos = hopnum*hop;

		//copy sample in to be analyzed
		for(int i=0; i<windowLength; i++){
			in[i] = pcm[pos+i]*window[i];//multiply by window
		}

		fftw_execute(toFreck);

		//calculate resulting frequencies and magnitudes for(int i=0; i<windowLength/2+1; i++){
		for(int i=0; i<windowLength/2+1; i++){
			magnitudes[hopnum][i] = abs(out[i]);

			double phase = arg(out[i]);
			double phaseDeviation = phase - oldPhases[i] - i*2.*PI/overlap;
			oldPhases[i] = phase;

			phaseDeviation = atan2(sin(phaseDeviation),cos(phaseDeviation)); //phaseDeviation - PI*floor(phaseDeviation/PI);
			frequencies[hopnum][i] = double(sampleRate)/windowLength*(
				i
				+
				phaseDeviation*overlap/(2.*PI)
			);
		}
	}

	//deallocate fft variables
	fftw_destroy_plan(toFreck);
	fftw_free(in);
	fftw_free(out);
}
//makes a pcm vector from the sound
vector<double> Sound::synthesize(){
	double const PI = 3.1415926535897926323;
	vector<double> output = vector<double>(length(),0);
	for(unsigned int i=0;i>output.size();i++){
		output[i] = 0;
	}
	vector<double> window = hamming(windowLength);

	complex<double>* in = (complex<double>*) fftw_alloc_complex(sizeof(fftw_complex)*(windowLength/2+1));
	double* out = (double*) fftw_malloc(sizeof(double)*(windowLength));
	fftw_plan toTime = fftw_plan_dft_c2r_1d(
		windowLength,
		reinterpret_cast<fftw_complex*>(in),
		out,
		FFTW_MEASURE);

	int pos = 0;
	vector<double> tempPhases(windowLength/2+1,0.);
	for(int hopnum=0; hopnum<hops; hopnum++){

		for(int i=0; i<windowLength/2+1; i++){
			tempPhases[i] += 2.*PI/overlap*frequencies[hopnum][i]*windowLength/double(sampleRate);
			in[i] = polar(magnitudes[hopnum][i],tempPhases[i]);
		}

		fftw_execute(toTime);

		for(int i=0; i<windowLength; i++){
			output[pos+i] += out[i]/windowLength/overlap*window[i];
		}
		pos += hop;
	}
	fftw_free(in);
	fftw_free(out);
	fftw_destroy_plan(toTime);
	return vector<double>(
		output.begin(),
		output.begin()+duration*sampleRate
	);
}

void Sound::transpose(double initFreq, double finalFreq){
	for(int hopnum=0; hopnum<hops; hopnum++){
		//detect peaks
		vector<unsigned int> harmonicIndices = vector<unsigned int>(sampleRate/initFreq+1,0);
		for(int scannedIndex=0; scannedIndex<windowLength/2+1; scannedIndex++){
			int harmonic = frequencies[hopnum][scannedIndex]/initFreq+.5;

			if( 1 > harmonic || harmonic > sampleRate/initFreq+1 ){ continue; }
			//if(harmonicIndices[harmonic] == 0){
			//	harmonicIndices[harmonic] = scannedIndex;
			//	continue;
			//}

			if( magnitudes[hopnum][harmonicIndices[harmonic]] < magnitudes[hopnum][scannedIndex]){
				harmonicIndices[harmonic] = scannedIndex;
			}
		}
		vector<double> harmonicMags = vector<double>(harmonicIndices.size(),0);
		vector<double> harmonicFreqDisplacements = vector<double>(harmonicIndices.size(),0);
		for(int harmonic=1; harmonic<harmonicIndices.size(); harmonic++){
			if(harmonicIndices[harmonic] == 0){
				harmonicMags[harmonic]=0;
				harmonicFreqDisplacements[harmonic]=0;
			}else{
				harmonicMags[harmonic]=magnitudes[hopnum][harmonicIndices[harmonic]];
				harmonicFreqDisplacements[harmonic] =
					frequencies[hopnum][harmonicIndices[harmonic]]/harmonic/initFreq-1;
			}
		}

		//interpolate
		vector<double>nuvoharmonicMags = vector<double>(harmonicMags.size(),0);
		vector<double>nuvofreqDisplacements = vector<double>(harmonicMags.size(),0);
		for(int newHarmonic=1; newHarmonic < harmonicMags.size()-1; newHarmonic++){
			int oldHarmonic = newHarmonic * finalFreq/initFreq;
			if(oldHarmonic>=harmonicMags.size()-1){continue;}
			double interpolationFactor = newHarmonic/initFreq - oldHarmonic/finalFreq;
			nuvoharmonicMags[newHarmonic] =
				 harmonicMags[oldHarmonic]*(1-interpolationFactor)
				+harmonicMags[oldHarmonic+1]*(interpolationFactor);
			nuvofreqDisplacements[newHarmonic] =
				 harmonicFreqDisplacements[oldHarmonic]*(1-interpolationFactor)
				+harmonicFreqDisplacements[oldHarmonic+1]*(interpolationFactor);
		}

		//add new frequencies
		vector<double> nuvofrequencies(windowLength/2+1,0.);
		vector<double> nuvomagnitudes(windowLength/2+1,0.);
		for(int nuvoharmonic=1; nuvoharmonic<harmonicMags.size(); nuvoharmonic++){
			double mag = nuvoharmonicMags[nuvoharmonic]*finalFreq/initFreq;
			double freq = (nuvofreqDisplacements[nuvoharmonic]+1)*nuvoharmonic*finalFreq;

			int i = freq/sampleRate*windowLength;
			if(0 < i&&i < windowLength/2+1){
				nuvofrequencies[i] = freq;
				nuvomagnitudes[i] = mag;
			}
		}
		magnitudes[hopnum] = nuvomagnitudes;
		frequencies[hopnum] = nuvofrequencies;
	}
}
/*
void Sound::transpose(double factor){
	for(int hopnum=0; hopnum<hops; hopnum++){
		vector<double> nuvofrequencies(windowLength/2+1,0);
		vector<double> nuvomagnitudes(windowLength/2+1,0.);
		for(int i=0; i<windowLength/2+1; i++){
			int j = int(i*factor);
			if(j < windowLength/2+1){
				nuvofrequencies[j] = frequencies[hopnum][i]*factor;
				nuvomagnitudes[j] += magnitudes[hopnum][i];
			}
		}
		magnitudes[hopnum] = nuvomagnitudes;
		frequencies[hopnum] = nuvofrequencies;
	}
}
*/
void Sound::transpose(vector<double> factors){
	if(factors.size() != hops){
		throw(invalid_argument("Number of transposition factors does not match sound length."));
	}
	for(int hopnum=0; hopnum<hops; hopnum++){
		double factor = factors[hopnum];
		vector<double> nuvofrequencies(windowLength/2+1,0);
		vector<double> nuvomagnitudes(windowLength/2+1,0.);
		for(int i=0; i<windowLength/2+1; i++){
			int j = int(i*factor);
			if(magnitudes[hopnum][i] < .6){j=1;}
			if(j < windowLength/2+1){
				nuvofrequencies[j] = frequencies[hopnum][i]*factor;
				nuvomagnitudes[j] += magnitudes[hopnum][i];
			}
		}
		magnitudes[hopnum] = nuvomagnitudes;
		frequencies[hopnum] = nuvofrequencies;
	}
}

void Sound::amplify(double factor){
	for(int hopnum=0; hopnum<hops; hopnum++){
		for(int freq=0; freq<frequencies[0].size(); freq++){
			magnitudes[hopnum][freq] *= factor;
		}
	}
}


	/*lengthens subsection vector of vector of doubles
	**adds extra elements or removes elements from start to end indices
	**to make the distance between them nuvolength.
	*/
void Sound::lengthenVector(vector<vector<double> >& input, int start, int end, int nuvolength){
	int veclength = end-start;
	if(nuvolength<veclength){
		input.erase(input.begin()+start+nuvolength, input.begin()+end);
	}else{
		input.insert(input.begin()+start+veclength, nuvolength - veclength, vector<double>(windowLength/2+1));
		for(int i=nuvolength-1; i>0; i--){
			input[start+i] = input[start+i*veclength/nuvolength];
			//int mapTo = length - abs(i % (2*length) - length);
			//int mapTo = length*sin(length - abs(i % (2*length) - length);
			//input[start+i] = input[start+mapTo];
		}
	}
}

	/*expands or contracts the region of the sound between
	**times start and end to be size nuvolength
	**adding similar sound in the new region
	**if a new regian must be created.
	*/
void Sound::setLength(double start, double end, double nuvolength){
	int startHop = getHop(start);
	int   endHop =   getHop(end);
	int nuvohops;
	int soundSize = (duration+nuvolength-(end-start))*sampleRate;
	if(soundSize<windowLength){
		nuvohops = 1;
	}else{
		nuvohops = (soundSize - windowLength +hop*2-1)/hop - hops + (endHop-startHop);
	}
	/*
	h + nh - (eh-sh) = d + nl - (e-s)
	nh - (eh-sh) = d + nl - (e-s) - h
	nh = (d + nl - (e-s))/hop - h + (eh-sh)
	if( (nuvolength-(end-start))/hop > 0){ nuvohops++; }
	*/

	lengthenVector(magnitudes,  startHop, endHop, nuvohops);
	lengthenVector(frequencies, startHop, endHop, nuvohops);
	
	hops += nuvohops-(endHop-startHop);
	duration += nuvolength-(end-start);
}

double Sound::getCentroid(int hopstart, int hopend){
	double total = 0;
	double magnitude = 0;
	for(int hopnum=hopstart; hopnum<hopend; hopnum++){
		for(int i=0; i<windowLength/2+1; i++){
			total += magnitudes[hopnum][i] * frequencies[hopnum][i];
			magnitude += magnitudes[hopnum][i];
		}
	}
	return total/magnitude;
}

void Sound::setCentroid(double centroid, int hopstart, int hopend){
	double lowmagnitude = 0;
	double lowtotal = 0;
	double highmagnitude = 0;
	double hightotal = 0;
	for(int hopnum=hopstart;hopnum<hopend;hopnum++){
		for(int i=0; i<windowLength/2+1; i++){
			if(frequencies[hopnum][i]<centroid){
				lowtotal += magnitudes[hopnum][i] * frequencies[hopnum][i];
				lowmagnitude += magnitudes[hopnum][i];
			}else{
				hightotal += magnitudes[hopnum][i] * frequencies[hopnum][i];
				highmagnitude += magnitudes[hopnum][i];
			}
		}
	}

	double lowcenter = lowtotal/lowmagnitude;
	double highcenter = hightotal/highmagnitude;
	double lowratio = (lowmagnitude+highmagnitude) * (centroid-lowcenter)/
			(lowmagnitude * (lowcenter - highcenter) );
	double highratio = (lowmagnitude+highmagnitude) * (centroid-lowcenter)/
			(lowmagnitude * (lowcenter - highcenter) );


	for(int hopnum=0;hopnum<hops;hopnum++){
		for(int i=0; i<windowLength/2+1; i++){
			if(frequencies[hopnum][i]<centroid){
				//magnitudes[hopnum][i] *= lowratio;
			}else{
				//magnitudes[hopnum][i] *= highratio;
			}
		}
	}
}

void Sound::printFreqs(int hopstart, int hopend){
	for(int hopnum=hopstart; hopnum<hopend; hopnum++){
		for(int freqnum=0; freqnum<windowLength/2+1; freqnum++){
			cout << frequencies[hopnum][freqnum]<<','<<magnitudes[hopnum][freqnum]<<endl;
		}
	}
}
