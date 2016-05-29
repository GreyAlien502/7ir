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
	return input;
}


int main(int args, char** argv){
	int windowSize = 2048;
	int overlap = 4;//windowSize/2;
	vector<double> input = fileio::read(argv[1]);
	input.erase(input.begin()+input.size(),input.end());

	cerr << "analyzing...";
	sound::Sound song = sound::Sound(input, overlap, windowSize);
	cerr << "done\n";

	cerr << "transposing...";
	song.transpose(1);
	cerr << "done\n";

	//vector<double> output = song.synthesize();
	//output.erase(output.begin(),output.begin()+windowSize/4);
//	sound::Sound song2 = sound::Sound(output,overlap, windowSize);


	int show = song.hops/100+1;
	for(int i=0; i<song.hops-4; i+=show){
		for(int j=0; j<song.windowLength/2+1; j+=1){
			printf("%d\t%f\t%f\n",
				i,
				song.frequencies[i][j],
				song.magnitudes[i][j]
					/*(
						pow(abs(
						(one-two - two-tri)
						),2)
						+
						pow(abs(
						((two-tri) - (tri-fou))
						),2)
					)/pow(abs(
						two-tri
					)*two,2)*/

				);
		}
	}


	/*if( fileio::save(song2.synthesize(),argv[2])){
		cerr<<"Saved\n";
/	}else{
		cerr<<"Error: failed to save\n";
		exit(1);
	}*/
}
