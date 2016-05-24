#include <vector>
#include <stdio.h>
#include <iostream>
#include <cmath>

#include "Sound.h"
#include "fileio.h"

using namespace std;
int samplerate = 44100;//Hz

void normalize(vector<double> vect){
	double top = 0;
	for(unsigned int i=0;i<vect.size();i++){
		if(abs(vect[i])>top){top=abs(vect[i]);}
	}
	cerr<<'t'<<top<<endl;
	for(unsigned int i=0;i<vect.size();i++){
		vect[i] /= top;
	}
	top = 0;
	for(unsigned int i=0;i<vect.size();i++){
		if(abs(vect[i])>top){top=abs(vect[i]);}
	}
	cerr<<"T"<<top<<endl;
}


int main(int args, char** argv){
	int windowSize = 2048;
	int overlap = 32;
	vector<double> input = fileio::read(argv[1]);

	sound::Sound song = sound::Sound(input, overlap, windowSize);
	int show = song.hops/10000+1;
	/*for(int i=0; i<song.hops; i+=show){
		for(int j=0; j<2048; j+=1){
			printf("%d\t%d\t%f\n", i,j,song.frequencies[i][j]);
		}
	}*/
	cerr << song.length()<<endl;
	vector<double> output = song.synthesize();
	normalize(output);
	cerr << output.size()<<endl;
	if( fileio::save(output,"output")){
		cerr<<"Saved\n";
	}else{
		cerr<<"Error: failed to save\n";
		exit(1);
	}
}
