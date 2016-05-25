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
	soundData = vector< vector<complex<double> > >(hops, vector<complex<double> >(windowLength/2+1));




	double in [windowLength];
	complex<double>* out = (complex<double>*) fftw_malloc(sizeof(fftw_complex)*(windowLength/2+1));
	fftw_plan toFreck = fftw_plan_dft_r2c_1d(
		windowLength,
		in,
		reinterpret_cast<fftw_complex*>(out),
		FFTW_MEASURE);

	int pos = 0;
	for(int hopnum=0; hopnum<hops; hopnum++){
		pos += hop;
		//if(pos%(length/100)==0){cout<<100*pos/(length)<<"%"<<endl;}

		copy(pcm.begin()+pos, pcm.begin()+pos+windowLength, in);
		for(int i=0; i<windowLength; i++){
			in[i] *= window[i];
		}

		fftw_execute(toFreck);

		copy(&out[0],&out[windowLength/2+1],soundData[hopnum].begin());
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

	//WINDOW ANALYSIS
	double pTot =0;
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
	cerr << "pSpread:"<<(pMax-pMin)/pSum*windowLength*100<<"%"<<endl;

	double out [windowLength];
	complex<double>* in = (complex<double>*) fftw_malloc(sizeof(fftw_complex)*(windowLength/2+1));
	fftw_plan toTime = fftw_plan_dft_c2r_1d(
		windowLength,
		reinterpret_cast<fftw_complex*>(in),
		out,
		FFTW_MEASURE);

	int pos = 0;
	for(int hopnum=0; hopnum<hops; hopnum++){
		pos += hop;

		copy(soundData[hopnum].begin(),soundData[hopnum].end(),in);

		fftw_execute(toTime);

		for(int i=0; i<windowLength; i++){
			output[pos+i] += out[i]/windowLength*window[i]/pMax;
		}
	}
	return output;
}


void sound::Sound::transpose(int amount){
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
}

void sound::Sound::lowpass(int amount){
	for(int i=0;i<hops;i++){
		for(int j=amount;j<windowLength/2+1;j++){
			soundData[i][j] = 0;
		}
	}
}
void sound::Sound::highpass(int amount){
	for(int i=0;i<hops;i++){
		for(int j=amount;j>0;j--){
			soundData[i][j] = 0;
		}
	}
}
