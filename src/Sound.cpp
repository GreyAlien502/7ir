#include <vector>
#include <complex>
#include <cmath>
#include <iostream>

#include <fftw3.h>

#include "Sound.h"

using namespace std;

vector<double> hamming(int windowLength) {
	double PI = 3.14159265358979323;
	cerr<<'e'<<endl;
	vector<double> output(windowLength);
	cerr<<'e'<<endl;
	for(int i = 0; i < windowLength/2; i++) {
		output[i] = 0.54 - 0.46 * cos( 2 * PI * (i / (double) (windowLength - 1) ));
		/*if(i<windowLength/4){
			output[windowLength/2+i] = sqrt(0.5 - 0.5 * cos( 2 * PI * (i / (double) (windowLength - 1) )));
			output[windowLength/2-i]=output[windowLength/2+i];
		}else if(i==windowLength/4){
			output[i] = sqrt(.5);
		}else{
			output[windowLength/2+i]=output[windowLength/2+i] = sqrt(1 - pow(output[windowLength-i],2) );
			output[windowLength/2-i]=output[windowLength/2+i];
		}*/
	}
	return output;
}





int sound::Sound::length(){
	return windowLength + hops*windowLength/overlap;
}

//make sound based off of input pcm data
sound::Sound::Sound(vector<double> pcm, int overlapFactor, int sizeOfWindow){
	double PI = 3.1415926535897926323;
	int length = pcm.size();
	vector<double> window = hamming(sizeOfWindow);

	//initialize object variables
	windowLength = sizeOfWindow;
	overlap = overlapFactor;
	hop = windowLength/overlap;
	hops = length/hop;
	magnitudes = 
	frequencies = 
		vector<vector<double> > (hops, vector<double>(windowLength/2+1));


	//variables to use for fft
	double in [windowLength];
	complex<double>* out = (complex<double>*) fftw_malloc(sizeof(fftw_complex)*(windowLength/2+1));
	fftw_plan toFreck = fftw_plan_dft_r2c_1d(
		windowLength,
		in,
		reinterpret_cast<fftw_complex*>(out),
		FFTW_MEASURE);


	vector<double> oldPhases = vector<double>(windowLength/2+1,0);//to calculate phase change
	int pos = 0;
	for(int hopnum=0; hopnum<hops; hopnum++){
		pos += hop;

		//copy sample in to be analyzed
		copy(pcm.begin()+pos, pcm.begin()+pos+windowLength, in);
		for(int i=0; i<windowLength; i++){
			in[i] *= window[i];
		}

		fftw_execute(toFreck);
		
		//calculate resulting frequencies and magnitudes
		for(int i=0; i<windowLength/2+1; i++){
			magnitudes[hopnum][i] = abs(out[i]);

			double phase = arg(out[i]);
			double phaseDeviation = phase - oldPhases[i] - (double)(i)*2.*PI*hop/windowLength;
			oldPhases[i] = phase;

			phaseDeviation = phaseDeviation - 2*PI*floor(phaseDeviation/2/PI);
			frequencies[hopnum][i] = 44100./windowLength*(i + phaseDeviation*overlap/(2.*PI) );
		}
	}

	//deallocate fft variables
	fftw_destroy_plan(toFreck);
	fftw_free(out);
}
/*
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

*/
void sound::Sound::transpose(double factor){
	for(int i=0; i<hops; i++){
		vector<double> nuvofrequencies(windowLength/2+1);
		vector<double> nuvomagnitudes(windowLength/2+1,0.);
		for(int j=0; j<windowLength/2+1; j++){
			int index = int(j*factor);
			if(index < windowLength/2+1){
				nuvofrequencies[index] = frequencies[i][j]*factor;
				nuvomagnitudes[index] += magnitudes[i][j];
			}
		}
		magnitudes[i] = nuvomagnitudes;
		frequencies[i] = nuvofrequencies;
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
}*/
