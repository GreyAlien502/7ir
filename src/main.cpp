#include <vector>
#include <stdio.h>
#include <iostream>
#include <cmath>

#include "Sound.h"
#include "fileio.h"

using namespace std;
int samplerate = 44100;//Hz
double factor = 1.1;

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

sound::Sound notify(string phoneme, int offset, int consonant, int cutoff, int length, int notenum){
	offset = offset*samplerate/1000;
	consonant = consonant*samplerate/1000;
	cutoff = cutoff*samplerate/1000;
	vector<double> pcm = fileio::read("voicelibrary/"+phoneme+".wav");
	sound::Sound consPart = sound::Sound( vector<double>(pcm.begin()+offset, pcm.begin()+consonant) );
	sound::Sound vowlPart = sound::Sound( vector<double>(pcm.begin()+consonant, pcm.end()-cutoff));
	
	length = length*samplerate/1000/consPart.hop;
	vowlPart.setHops(length);
	consPart.append(vowlPart);

	double freq = 440.0 * pow(2.0, (notenum - 69)/12);
	consPart.transpose(freq/349.228);

	return sound::Sound(consPart);
}


int main(int args, char** argv){
	int windowSize = 2048;
	int overlap = 4;

	cerr << "analyzing...";
	sound::Sound song = notify("わ",35,115,205,800,70);
	sound::Sound song2 = notify("し",25,117,99,800,70);
	song.append(song2);
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
	int show = song.hops/100+1;
	for(int i=0; i<song.hops-4; i+=show){
		for(int j=0; j<windowSize; j+=3){
			printf("%f\t%d\t%f\n",
				song.frequencies[i][j],
				i,
				song.magnitudes[i][j]
				);
		}
	}

	cerr << "transposing...";
	song.transpose(factor);
	cerr << "done.\n";

	/*cerr << "filtering...";
	song.lowpass(2000);
	cerr << "done.\n";*/


	cerr << "synthesizing...";
	vector<double> output = song.synthesize();
	cerr << "done.\n";


	output = normalize(output);

	if(fileio::save(output,"output.wav")){
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
