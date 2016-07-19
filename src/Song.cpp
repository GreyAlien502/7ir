#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "Song.h"

using namespace std;

song::Song::Song(string path){
	ifstream ust(path);
	string line;
	if(ust.is_open()){
		while(line != "[#SETTING]"){
			getline(ust, line);
		}
		map<string,string> parameters = song::parameters(ust);
		
		tempo = stod(parameters["Tempo"]);
		projectName = parameters["ProjectName"];
		outFile = parameters["OutFile"];
		voiceDir = parameters["VoiceDir"];

		getline(ust,line);
		while(line != "[#TRACKEND]"){
			notes.push_back(Note(ust));
			getline(ust,line);
		}
	}else{
		cerr<<"Couldn't open ust file\n";
		exit(1);
	}
}

vector<double> song::Song::synthesize(voiceLibrary::VoiceLibrary library){
	int sampleRate = library.sampleRate;
	int hop = library.hop;
	vector<double> output = vector<double>();
	for(int i=0; i<notes.size(); i++){
		sound::Sound noteSound= notes[i].getSound(library);
		/*if(i+1 < notes.size()){
			if(notes[i].duration >
					notes[i+1].delta
					- notes[i+1].getPhone(library).preutter*hop/sampleRate*1000.){
				//TODO:smoothe overlapped area
				cerr<<"ignoring overlap\n";
			}
		}*/
		vector<double> pcm = noteSound.synthesize();

		int startTime;// start time in samples
		if(i==0){startTime = notes[i].getPhone(library).preutter*hop/sampleRate*1000.;}
		else{startTime += notes[i].delta/1000.*sampleRate;}

		cerr<<startTime+notes[i].duration/1000.*sampleRate<<endl;
		cerr<<startTime+pcm.size()<<endl;

		//allocate new length for output
		output.resize(startTime+pcm.size(),0.);

		//add sound.synth to output starting where it needs to start
		for(int j=0; j<pcm.size(); j++){
			output[startTime+j] += pcm[j];
		}
	}
	return output;
}
