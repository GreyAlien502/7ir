#include <vector>
#include <stdio.h>
#include <iostream>
#include <cmath>

#include "Sound.h"
#include "fileio.h"

using namespace std;
int samplerate = 44100;//Hz
double factor = .5;

vector<double> normalize(vector<double> input){
	double top =0;
	for(unsigned int i=0;i<input.size();i++){
		if(abs(input[i])>top){
			top=abs(input[i]);
		}
	}
	for(unsigned int i=0;i<input.size();i++){
		input[i] /= top;
	}
	return input;
}


int main(int args, char** argv){
	int windowSize = 2048;
	int overlap = 4;
	vector<double> input = fileio::read(argv[1]);
	input.erase(input.begin()+input.size(),input.end());

	cerr << "analyzing...";
	sound::Sound song = sound::Sound(input, overlap, windowSize);
	cerr << "done.\n";

	vector<double> factors = vector<double>(song.hops);
	for(int i=0; i<song.hops; i++){
		double hopfreq = double(song.hop)/samplerate;
		if(i<song.hops/2){
			factors[i] = 1+.03*pow(2,cos(i*hopfreq*43))-.03;
		}else{
			factors[i] = 1+.03*cos(i*hopfreq*43);
		}
	}

	cerr << "transposing...";
	song.transpose(factors);
	cerr << "done.\n";

	cerr << "filtering...";
	//song.lowpass(2000);
	cerr << "done.\n";



	cerr << "synthesizing...";
	vector<double> output = song.synthesize();
	cerr << "done.\n";


	int show = song.hops/100+1;
	for(int i=0; i<song.hops-4; i+=show){
		for(int j=0; j<song.windowLength/2+1; j+=1){
			printf("%d\t%f\t%f\n",
				i,
				song.frequencies[i][j],
				song.magnitudes[i][j]
				);
		}
	}
	output = normalize(output);

	if(fileio::save(output,argv[2])){
		cerr<<"Saved\n";
	}else{
		cerr<<"Error: failed to save\n";
		exit(1);
	}
	




/*QUALITY CONTROL
	vector<double> outpuu = vector<double> (input.size(),0);
	vector<float> inpt = vector<float>(input.size(),0.);
	for(int i=0; i<input.size(); i++){
		inpt[i] = float(input[i]);	
	}
	vector<float>outpt = vector<float>(input.size(),0.);
	smbPitchShift(factor,inpt.size(),windowSize,overlap,44100,&inpt[0],&outpt[0]);
	for(int i=0; i<input.size(); i++){
		outpuu[i] = double(outpt[i]);	
	}
	outpuu=normalize(outpuu);
	if(fileio::save(outpuu,"out2")){
		cerr<<"Saved\n";
	}else{
		cerr<<"Error: failed to save\n";
		exit(1);
	}
*/
}
