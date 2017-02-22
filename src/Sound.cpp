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


//make sound based off of input pcm data
Sound::Sound(vector<double> pcm, int overlap, int sizeOfWindow, int rate){
	if(sizeOfWindow%overlap != 0){
		throw invalid_argument("Window size munt be a multiple of overlap factor.(");
	}

	double const PI = 3.14159265358979323846;
	vector<double> window = hamming(sizeOfWindow);

	//initialize object variables
	sampleRate = rate;
	windowLength = sizeOfWindow;
	hop = windowLength/overlap;
	hops = pcm.size()/hop+1;

	duration = double(pcm.size())/rate;
	pcm.insert(pcm.begin(),	windowLength/2, 0);
	pcm.resize((windowLength+hops-1)*hop);

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
vector<double> Sound::rawSynthesize(){
	int overlap = windowLength/hop;
	double const PI = 3.1415926535897926323;
	vector<double> output = vector<double>(hops*hop+windowLength,0);
	for(int i=0;i>output.size();i++){
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
	return output;
}

vector<double> Sound::synthesize(){
	vector<double> output = rawSynthesize();
	return vector<double>(
		output.begin()+windowLength/2,
		output.begin()+windowLength/2+duration*sampleRate
	);
}
