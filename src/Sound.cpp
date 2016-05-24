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
		output[i] = 0.54 - 0.46 * cos( 2 * PI * (i / (double) (windowLength - 1) ));
	}
	return output;
}

double magnitude(double* array){
	return sqrt(pow(array[0],2) + pow(array[1],2));
}




int sound::Sound::length(){
	return windowLength + hops*windowLength/overlap;
}

sound::Sound::Sound(vector<double> pcm, int overlapFactor, int sizeOfWindow){
	windowLength = sizeOfWindow;
	overlap = overlapFactor;
	vector<double> window = hamming(windowLength);
	int length = pcm.size();

	hop = windowLength/overlap;
	hops = length/hop;
	soundData = vector< vector<complex<double> > >(length/hop, vector<complex<double> >(windowLength));

	for(int i=0;i<hop;i++){
		double pSum =0;
		for(int j=i;j<windowLength;j+=hop){
			pSum += window[j]*window[j];
		}
		printf("%d\t%f\n", i, pSum);
	}

	double in [windowLength];
	complex<double>* out = (complex<double>*) fftw_malloc(sizeof(fftw_complex)*(windowLength/2+1));
	fftw_plan toFreck = fftw_plan_dft_r2c_1d(
		windowLength,
		in,
		reinterpret_cast<fftw_complex*>(out),
		FFTW_MEASURE);

	for(int hopnum=0; hopnum<hops; hopnum++){
		int pos = hopnum * hop;
		//if(pos%(length/100)==0){cout<<100*pos/(length)<<"%"<<endl;}

		copy(pcm.begin()+pos, pcm.begin()+pos+windowLength, in);
		for(int i=0; i<windowLength; i++){
			in[i] *= window[i];
		}

		fftw_execute(toFreck);

		//copy(out, out+windowLength+2, frequency.begin()+pos);
		for(int i=0; i<windowLength; i++){
			soundData[hopnum][i] = (complex<double>)(out[i]);//magnitude(out[i]);
		}
	}

	fftw_destroy_plan(toFreck);
	fftw_free(out);
}

vector<double> sound::Sound::synthesize(){
	vector<double> output = vector<double>(length());
	for(unsigned int i=0;i>output.size();i++){
		output[i] = 0;
	}
	vector<double> window = hamming(windowLength);

	double out [windowLength];
	complex<double>* in = (complex<double>*) fftw_malloc(sizeof(fftw_complex)*(windowLength/2+1));
	fftw_plan toTime = fftw_plan_dft_c2r_1d(
		windowLength,
		reinterpret_cast<fftw_complex*>(in),
		out,
		FFTW_MEASURE);

	for(int hopnum=0; hopnum<hops; hopnum++){
		int pos = hopnum * hop;

		for(int i=0; i<windowLength/2+1; i++){
			in[i] = soundData[hopnum][i];//*cos(phases[hopnum][i]);
		}

		fftw_execute(toTime);

		for(int i=0; i<windowLength; i++){
			output[pos+i] = out[i]/windowLength*window[i]*100. /.0327939/.07;
		}
	}
	return output;
}
