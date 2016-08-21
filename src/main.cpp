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
	voiceLibrary::VoiceLibrary teto = voiceLibrary::VoiceLibrary(
		"tetoreal/重音テト音声ライブラリー/重音テト単独音",
		8, //overlap
		1024 //windowSize
	);
	cerr << "...done.\n";

	cerr << "loading song...";
	song::Song sang = song::Song("youka_no_onnna.ust");
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
