#include <vector>
#include <complex>
#include <cmath>
#include <iostream>

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





int sound::Sound::length(){
	return hops*hop+windowLength;
}

//make sound based off of input pcm data
sound::Sound::Sound(vector<double> pcm, int overlapFactor, int sizeOfWindow){
	double PI = 3.14159265358979323846;
	int length = pcm.size();
	vector<double> window = hamming(sizeOfWindow);

	//initialize object variables
	windowLength = sizeOfWindow;
	overlap = overlapFactor;
	hop = windowLength/overlap;
	hops = (length-windowLength)/hop;
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
			frequencies[hopnum][i] = 44100./windowLength*(
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
vector<double> sound::Sound::synthesize(){
	double const PI = 3.1415926535897926323;
	vector<double> output = vector<double>(length(),0);
	for(unsigned int i=0;i>output.size();i++){
		output[i] = 0;
	}
	vector<double> window = hamming(windowLength);

	//WINDOW ANALYSIS
	/*double pTot =0;
	double pMax =0;
	double pMin =windowLength*100;
	double pSum;
	for(int i=0;i<hop;i++){
		pSum=0;
		for(int j=i;j<windowLength;j+=hop){
			pSum += window[j]*window[j];
		}
		pTot +=pSum;
		if(pSum>pMax){ pMax=pSum; }
		if(pSum<pMin){ pMin=pSum; }
	}
	cerr << "pSpread:"<<(pMax-pMin)/pSum*windowLength*100<<"%"<<endl;*/

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
		pos += hop;

		for(int i=0; i<windowLength/2+1; i++){
			tempPhases[i] += 2.*PI/overlap*frequencies[hopnum][i]*windowLength/44100.;
			in[i] = polar(magnitudes[hopnum][i],tempPhases[i]);
		}

		fftw_execute(toTime);

		for(int i=0; i<windowLength; i++){
			output[pos+i] += out[i]/windowLength*window[i];
		}
	}
	fftw_free(in);
	fftw_free(out);
	fftw_destroy_plan(toTime);
	return output;
}

void sound::Sound::transpose(double factor){
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
void sound::Sound::transpose(vector<double> factors){
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

//Appends a sound object to another, probably not very fast or useful 
void sound::Sound::append(Sound sound2){
	if( sound2.windowLength != windowLength ||
			sound2.overlap != overlap){
		throw(invalid_argument("Incompatible Sound objects"));
	}
	hops += sound2.hops;
	magnitudes.insert(
		magnitudes.end(),
		sound2.magnitudes.begin(),
		sound2.magnitudes.end());
	frequencies.insert(
		frequencies.end(),
		sound2.frequencies.begin(),
		sound2.frequencies.end());
}

void sound::Sound::setHops(unsigned int nuvohops){
	vector< vector<double> > nuvomagnitudes = vector<vector<double> > (nuvohops, vector<double>(windowLength/2+1,0.));
	vector< vector<double> > nuvofrequencies = vector<vector<double> > (nuvohops, vector<double>(windowLength/2+1,0.));
	for(int hopnum=0; hopnum<nuvohops; hopnum++){
		double virtualOldHopnum = double(hopnum)*nuvohops/hops;
		int oldHopnum = floor(virtualOldHopnum);
		double phase = virtualOldHopnum - oldHopnum;
		if(oldHopnum +1 >= hops){
			oldHopnum = hops-2;
			phase = 0;
		}
		for(int i=0; i<windowLength/2+1; i++){
			nuvomagnitudes[hopnum][i] = magnitudes[oldHopnum][i]*(1-phase) + magnitudes[oldHopnum+1][i]*(phase);
			nuvofrequencies[hopnum][i] = frequencies[oldHopnum][i]*(1-phase) + frequencies[oldHopnum+1][i]*(phase);
		}
	}
	magnitudes = nuvomagnitudes;
	frequencies = nuvofrequencies;
	hops = nuvohops;
}
	

void sound::Sound::lowpass(double frequency){
	for(int hopnum=0;hopnum<hops;hopnum++){
		for(int i=0;i<windowLength/2+1;i++){
			if(frequencies[hopnum][i]>frequency){
				magnitudes[hopnum][i] =0;
			}
		}
	}
}
