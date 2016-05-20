#include <vector>
#include <stdio.h>

#include "Sound.h"
#include "fileio.h"

using namespace std;
int samplerate = 44100;//Hz

int main(int args, char** argv){
	vector<double> input = fileio::read(argv[1]);
	sound::Sound song = sound::Sound(input);
	int show = song.hops/100+1;
	for(int i=0; i<song.hops; i+=show){
		for(int j=0; j<2048; j+=1){
		printf("%d\t%d\t%f\n", i,j,song.frequencies[i][j]);
		}
	}
	/*
	if(fileio::save(transposee ,argv[2])){
		printf("Copied\n");
	}else{
		printf("Error\n");
	}
	*/
}
