#include <stdlib.h>
#include <stdio.h>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <string.h>
#include <fftw3.h>
#include <vector>
#include <algorithm>

#include "fileio.h"

using namespace std;
int samplerate = 44100;//Hz

void hamming(int windowLength, float *buffer) {
	double PI = 3.14159265358979323;
	for(int i = 0; i < windowLength; i++) {
		buffer[i] = 0.54 - (0.46 * cos( 2 * PI * (i / ((windowLength - 1) * 1.0))));
	}
}

vector<double> cutoff(vector<double> sound, int frequency, int tightness){
	int length = sound.size();
	vector<double> output(length);
	double in [tightness];
	double out [tightness];
	fftw_complex* mid = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(tightness/2+1));
	//fftw_complex* mid2 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(tightness/2+1));
	if(NULL == mid){
		printf("moremem");
		exit(1);
	}
	fftw_plan toFreck = fftw_plan_dft_r2c_1d(tightness,in,mid,FFTW_ESTIMATE);
	fftw_plan toTime  = fftw_plan_dft_c2r_1d(tightness,mid,out,FFTW_ESTIMATE);
	cout << "working..."<<endl;
	cout <<length<<endl;
	int hop = tightness;
	for(int pos=0; pos+hop<length; pos+=hop){
		if(pos%(length/100)==0){cout<<100*pos/(length)<<endl;}
		copy(sound.begin()+pos, sound.begin()+pos+tightness, in);
		fftw_execute(toFreck);

	
		for(int i=0; i<tightness/2+1-frequency; i++){
			//cout<<i<<endl;
				mid[i][0] = mid[i+frequency][0];
				mid[i][1] = mid[i+frequency][1];
		}

		fftw_execute(toTime);
		copy(out, out+hop, output.begin()+pos);
	}
	cout << "done!\n";
	for(int i=0; i<length; i++){
		output[i] /= tightness;
	}

	fftw_destroy_plan(toFreck);
	fftw_destroy_plan(toTime);
	fftw_free(mid);
	return output;
}

int main(int args, char** argv){
	vector<double> copiee = fileio::read(argv[1]);
	vector<double> transposee = cutoff(copiee,3,20480);
	transposee.insert(transposee.end(),copiee.begin(),copiee.end());
	if(fileio::save(transposee ,argv[2])){
		printf("Copied\n");
	}else{
		printf("Error\n");
	}
}
