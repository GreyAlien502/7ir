#include <vector>
#include <complex>
#include <cmath>
#include <iostream>
#include <stdexcept>

#include <fftw3.h>

#include "Sound.h"

using namespace std;

vector<float> hamming(int windowLength) {
	float PI = 3.14159265358979323;
	vector<float> output(windowLength);
	for(int i = 0; i < windowLength; i++) {
		output[i] = .5 - .5 * cos( 2 * PI * (i / (float) (windowLength) ));
	}
	return output;
}


//make sound based off of input pcm data
Sound::Sound(vector<float> pcm, int overlap, int sizeOfWindow, int rate){
	if(sizeOfWindow%overlap != 0){
		throw invalid_argument("Window size munt be a multiple of overlap factor.(");
	}

	float const PI = 3.14159265358979323846;
	vector<float> window = hamming(sizeOfWindow);

	//initialize object variables
	sampleRate = rate;
	windowLength = sizeOfWindow;
	hop = windowLength/overlap;
	hops = pcm.size()/hop+1;

	duration = float(pcm.size())/rate;
	pcm.insert(pcm.begin(),	windowLength/2, 0);
	pcm.resize((windowLength+hops-1)*hop);

	magnitudes = vector<vector<float> > (hops, vector<float>(windowLength/2+1,0.));
	frequencies = vector<vector<float> > (hops, vector<float>(windowLength/2+1));


	//variables to use for fft
	float* in = (float*) fftwf_malloc(sizeof(float)*(windowLength));
	complex<float>* out = (complex<float>*) fftwf_alloc_complex(sizeof(fftwf_complex)*(windowLength/2+1));
	fftwf_plan toFreck = fftwf_plan_dft_r2c_1d(
		windowLength,
		in,
		reinterpret_cast<fftwf_complex*>(out),
		FFTW_MEASURE);


	vector<float> oldPhases = vector<float>(windowLength,0);//to calculate phase change
	int pos = 0;
	for(int hopnum=0; hopnum<hops; hopnum++){
		pos = hopnum*hop;

		//copy sample in to be analyzed
		for(int i=0; i<windowLength; i++){
			in[i] = pcm[pos+i]*window[i];//multiply by window
		}

		fftwf_execute(toFreck);

		//calculate resulting frequencies and magnitudes for(int i=0; i<windowLength/2+1; i++){
		for(int i=0; i<windowLength/2+1; i++){
			magnitudes[hopnum][i] = abs(out[i]);

			float phase = arg(out[i]);
			float phaseDeviation = phase - oldPhases[i] - i*2.*PI/overlap;
			oldPhases[i] = phase;

			phaseDeviation = atan2(sin(phaseDeviation),cos(phaseDeviation)); //phaseDeviation - PI*floor(phaseDeviation/PI);
			frequencies[hopnum][i] = float(sampleRate)/windowLength*(
				i
				+
				phaseDeviation*overlap/(2.*PI)
			);
		}
	}

	//deallocate fft variables
	fftwf_destroy_plan(toFreck);
	fftwf_free(in);
	fftwf_free(out);
}
//make a compatible sample
Sound Sound::compatibleSound(vector<float> pcm){
	return Sound(pcm, windowLength/hop, windowLength, sampleRate);
}

//makes a pcm vector from the sound
vector<float> Sound::rawSynthesize(){
	int overlap = windowLength/hop;
	float const PI = 3.1415926535897926323;
	vector<float> output = vector<float>(hops*hop+windowLength,0);
	for(int i=0;i>output.size();i++){
		output[i] = 0;
	}
	vector<float> window = hamming(windowLength);

	complex<float>* in = (complex<float>*) fftwf_alloc_complex(sizeof(fftwf_complex)*(windowLength/2+1));
	float* out = (float*) fftwf_malloc(sizeof(float)*(windowLength));
	fftwf_plan toTime = fftwf_plan_dft_c2r_1d(
		windowLength,
		reinterpret_cast<fftwf_complex*>(in),
		out,
		FFTW_MEASURE);

	int pos = 0;
	vector<float> tempPhases(windowLength/2+1,0.);
	for(int hopnum=0; hopnum<hops; hopnum++){

		for(int i=0; i<windowLength/2+1; i++){
			tempPhases[i] += 2.*PI/overlap*frequencies[hopnum][i]*windowLength/float(sampleRate);
			in[i] = polar(magnitudes[hopnum][i],tempPhases[i]);
		}

		fftwf_execute(toTime);

		for(int i=0; i<windowLength; i++){
			output[pos+i] += out[i]/windowLength/overlap*window[i];
		}
		pos += hop;
	}
	fftwf_free(in);
	fftwf_free(out);
	fftwf_destroy_plan(toTime);
	return output;
}

vector<float> Sound::synthesize(){
	vector<float> output = rawSynthesize();
	return vector<float>(
		output.begin()+windowLength/2,
		output.begin()+windowLength/2+duration*sampleRate
	);
}
