#include <vector>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <algorithm> 

#include "Song.h"
#include "fileio.h"

using namespace std;

int main(int args, char** argv){
	cerr << "loading voice library...";
	VoiceLibrary teto = VoiceLibrary(
		"tetoreal/重音テト音声ライブラリー/重音テト単独音",
		16, //overlap
		1024 //windowSize
	);
	cerr << "...done.\n";

/*
	sound::Sound nge = teto.getPhone("ク").sample;
	for(int i=0; i<nge.hops; i++){
		cout<< nge.getCentroid(i)<<endl;
		nge.setCentroid(
			i,
			+(2000-4500)/nge.hops*i + 4500.
		);
	}

	vector<double> pcm = nge.synthesize();
	for(int i=0; i<pcm.size(); i++){
		pcm[i] *= .3;
	}
	fileio::write(pcm,"output.wav");
*/


	cerr << "loading song...";
	Song sang = Song("kek.ust");
	cerr << "done.\n";

	cerr << "synthesizing...";
	sang.synthesize(teto, "output.wav");
	cerr << "done.\n";

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
