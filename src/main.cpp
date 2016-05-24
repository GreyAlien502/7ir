#include <vector>
#include <stdio.h>
#include <iostream>
#include <cmath>

#include "Sound.h"
#include "fileio.h"

using namespace std;
int samplerate = 44100;//Hz

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
}


int main(int args, char** argv){
	int windowSize = 2048;
	int overlap = 4;// windowSize/8;
	vector<double> input = fileio::read(argv[1]);

	cerr << "analyzing...";
	sound::Sound song = sound::Sound(input, overlap, windowSize);
	cerr << "done\n";

	 int show = song.hops/10000+1;
	for(int i=0; i<song.hops; i+=show){
		for(int j=0; j<song.windowLength/2+1; j+=1){
			printf("%d\t%d\t%f\n", i,j,abs(song.soundData[i][j]));
		}
	}

	cerr << "transposing...";
	song.transpose(15);
	cerr << "done\n";
	cerr<<song.soundData.size()<<endl;
	cerr<<song.soundData[10].size()<<endl;

	cerr << "synthesizing...";
	vector<double> output = song.synthesize();
	cerr << "done\n";

	cerr << song.soundData[0].size()<<endl;
	if( fileio::save(output,argv[2])){
		cerr<<"Saved\n";
	}else{
		cerr<<"Error: failed to save\n";
		exit(1);
	}
}
