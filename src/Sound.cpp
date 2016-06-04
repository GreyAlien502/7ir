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
	hops = length/hop;
	magnitudes = vector<vector<double> > (hops, vector<double>(windowLength/2+1,0.));
	frequencies = vector<vector<double> > (hops, vector<double>(windowLength/2+1));


	//variables to use for fft
	fftw_complex* in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(windowLength));
	complex<double>* out = (complex<double>*) fftw_malloc(sizeof(fftw_complex)*(windowLength));
	fftw_plan toFreck = fftw_plan_dft_1d(
		windowLength,
		in,
		reinterpret_cast<fftw_complex*>(out),
		FFTW_FORWARD,
		FFTW_MEASURE);


	vector<double> oldPhases = vector<double>(windowLength,0);//to calculate phase change
	int pos = 0;
	for(int hopnum=0; hopnum<hops; hopnum++){
		pos += hop;

		//copy sample in to be analyzed
		for(int i=0; i<windowLength; i++){
			in[i][0] = pcm[pos+i]*window[i];
			in[i][1] = 0;
		}
		if(hopnum==0){
			cerr<<in [22]<<endl;
		}

		fftw_execute(toFreck);

		if(hopnum==0){
			cerr<<out[22].imag()<<endl;
		}

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

	complex<double>* in = (complex<double>*) fftw_malloc(sizeof(fftw_complex)*(windowLength));
	fftw_complex* out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(windowLength));
	fftw_plan toTime = fftw_plan_dft_1d(
		windowLength,
		reinterpret_cast<fftw_complex*>(in),
		out,
		FFTW_BACKWARD,
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
			output[pos+i] += out[i][0]/windowLength*window[i];
		}
	}
	fftw_free(in);
	fftw_free(out);
	fftw_destroy_plan(toTime);
	return output;
}

void sound::Sound::transpose(double factor){
	for(int hopnum=0; hopnum<hops; hopnum++){
		vector<double> nuvofrequencies(windowLength,0);
		vector<double> nuvomagnitudes(windowLength,0.);
		for(int i=0; i<windowLength; i++){
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

/*
	if(amount<0){
		amount *=-1;
		for(int i=0; i<hops/amount; i++){
			soundData[i]=soundData[amount*i];
		}
		soundData.erase(soundData.begin()+hops/amount,soundData.end());
		hops = soundData.size();
	}else{
		soundData.resize(hops*amount);
		for(int i=0; i<hops; i++){
			fill(soundData.begin()+amount*(hops-1-i), soundData.begin()+amount*(hops-i), soundData[hops-i]);
		}
		hops *= amount;
	}
}*/

void sound::Sound::lowpass(double frequency){
	for(int hopnum=0;hopnum<hops;hopnum++){
		for(int i=0;i<windowLength/2+1;i++){
			if(frequencies[hopnum][i]>frequency){
				magnitudes[hopnum][i] =0;
			}
		}
	}
}
