#include <vector>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <algorithm> 

#include "VoiceLibrary.h"
#include "fileio.h"

using namespace std;
int samplerate = 44100;//Hz

double detectFrequency(vector<double> pcm){
	/*
		unsigned int crosses = 0;
		for(int i;i<length-1;i++){
			if(pcm[i]<0 && pcm[i+1]>0){
				crosses ++;
			}
		}
		return double(crosses)/length*samplerate;
		*/
	unsigned int length = pcm.size();
	int minPeriod = floor(samplerate/1046.5);
	int maxPeriod = floor(samplerate/82.407);
	vector<double> errors(maxPeriod-minPeriod,0);//deviations from periodicity for each period
	for(int period=minPeriod; period<maxPeriod; period++){
		int periods = length/period;//number of periods that fit in the sample
		for(int periodsIn=0; periodsIn<periods-1; periodsIn++){
			for(int t=0;t<period;t++){
				errors[period-minPeriod] += pow( pcm[period*(periodsIn) + t] * pcm[period*(periodsIn+1) + t],2);
			}
		}
		errors[period-minPeriod] /= (periods-1)*period;
		cout << double(samplerate)/period <<'\t'<< errors[period-minPeriod]<<endl;
	}
	return double(samplerate)/(minPeriod+distance( errors.begin(), max_element(errors.begin(),errors.end()) ));
}

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
	/*
		for(int hop=0; hop<vowlPart.hops; hop++){
			vector<double> hopdat = vowlPart.magnitudes[hop];
			for(int i=0;i<2;i++){
				int maxi = distance(hopdat.begin(), max_element(hopdat.begin(),hopdat.end()));
				//cout<<hop<<'\t'<< vowlPart.frequencies[hop][maxi]<<endl;
				hopdat[maxi] = -1;
			}
		}
		*/
	double frequency = detectFrequency(vector<double>(pcm.begin()+consonant, pcm.end()-cutoff));
	
	length = length*samplerate/1000/consPart.hop;


	double freq = 440.0 * pow(2.0, (notenum - 69.)/12);
	cerr<<"freq"<<frequency<<endl;
	vowlPart.transpose(freq/frequency);
	vowlPart.setHops(length);
	consPart.append(vowlPart);

	return sound::Sound(consPart);
}


int main(int args, char** argv){
	int windowSize = 2048;
	int overlap = 4;

	cerr << "analyzing...";
	voiceLibrary::VoiceLibrary teto = voiceLibrary::VoiceLibrary("teto");
	cerr << "done.\n";
	sound::Sound song = teto.getPhone("わ").sample;

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
		for(int j=0; j<windowSize; j+=4){
			/*cout <<
				i << '\t'<<
				song.frequencies[i][j] << '\t' <<
				song.magnitudes[i][j] << endl;*/
		}
	}

	/*cerr << "transposing...";
	song.transpose(factor);
	cerr << "done.\n";

	cerr << "filtering...";
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
