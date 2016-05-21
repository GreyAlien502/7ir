#include <vector>
#include <stdio.h>
#include <iostream>

#include "Sound.h"
#include "fileio.h"

using namespace std;
int samplerate = 44100;//Hz

void normalize(vector<double> vect){
	double top = 0;
	for(unsigned int i=0;i<vect.size();i++){
		if(vect[i]>vect[top]){top=i;}
	}
	for(unsigned int i=0;i<vect.size();i++){
		vect[i] /= vect[top];
	}
}


int main(int args, char** argv){
	vector<double> input = fileio::read(argv[1]);
	sound::Sound song = sound::Sound(input, 8, 2048);
	int show = song.hops/100+1;
	for(int i=0; i<song.hops; i+=show){
		for(int j=0; j<2048; j+=1){
			//printf("%d\t%d\t%f\n", i,j,song.frequencies[i][j]);
		}
	}
	cout << song.length()<<endl;
	vector<double> output = song.synthesize();
	normalize(output);
	cout << output.size()<<endl;
	fileio::save(output,"output");
	/*
	if(fileio::save(transposee ,argv[2])){
		printf("Copied\n");
	}else{
		printf("Error\n");
	}
	*/
}
