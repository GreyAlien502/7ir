#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "Song.h"
#include "fileio.h"

using namespace std;

song::Song::Song(string path){
	ifstream ust(path);
	string line;
	if(ust.is_open()){
		while(line != "[#SETTING]"){
			getline(ust, line);
			cerr<<line<<endl;
		}
		map<string,string> parameters = song::parameters(ust);
		
		tempo = stod(parameters["Tempo"]);
		projectName = parameters["ProjectName"];
		outFile = parameters["OutFile"];
		voiceDir = parameters["VoiceDir"];

		getline(ust,line);
		while(line != "[#TRACKEND]"){
			cerr<<line<<endl;
			notes.push_back(Note(ust));
			getline(ust,line);
		}
	}else{
		cerr<<"Couldn't open ust file\n";
		exit(1);
	}
}

void correlate(sound::Sound& sound1, sound::Sound& sound2, int start, int duration){
		/*if(i+1 < notes.size()){
			if(notes[i].duration >
					notes[i+1].delta
					- notes[i+1].getPhone(library).preutter*hop/sampleRate*1000.){
				//TODO:smoothe overlapped area
				cerr<<"ignoring overlap\n";
			}
		}*/
}

void song::Song::synthesize(voiceLibrary::VoiceLibrary library, string filename){
	int sampleRate = library.sampleRate;
	int hop = library.hop;
	
	if(notes.size()==1){
		cerr<<notes[0].lyric<<endl;
		fileio::write(
			notes[0].getSound(library).synthesize(),
			filename
		);
	}else{
		vector<double> prepcm = vector<double>();
		sound::Sound sound = notes[0].getSound(library);
		sound::Sound postsound;


		for(int i=0; i<notes.size(); i++){
			cerr<<notes[i].lyric<<endl;
			if(i<notes.size()-1){
				cerr<<"Ni+1.sound\n";
				postsound = notes[i+1].getSound(library);
				
				/*cerr<<"correlate Ni, Ni+1\n";
				correlate(
					sound,
					postsound,
					notes[i].getPhone(library).preutter
						+ notes[i+1].delta
						- notes[i+1].getPhone(library).preutter,
					notes[i+1].getPhone(library).preutter
				);*/
			}

			cerr<<"Ni.synth\n";
			vector<double> pcm = sound.synthesize();

			cerr<<"add Ni-1, Ni\n";
			if(prepcm.size() < pcm.size()){//expand prepcm if needed
				prepcm.resize(pcm.size(), 0);
			}

			for(int j=0; j<pcm.size(); j++){
				prepcm[j] += pcm[j];
			}


			cerr<<"write part Ni-1,i\n";
			int writeLength;
			if(i<notes.size()-1){
				writeLength = 
					notes[i].getPhone(library).preutter
					+ notes[i+1].delta/1000.*sampleRate/hop
					- notes[i+1].getPhone(library).preutter;
			}else{
				writeLength = 
					notes[i].getPhone(library).preutter
					+ notes[i].duration/1000.*sampleRate/hop;
			}
			fileio::append(
				vector<double>(
					prepcm.begin(),
					prepcm.begin() + hop*(writeLength)
				),
				filename
			);

			cerr<<"SHIFT\n";
			if(hop*(writeLength)<prepcm.size()){
				prepcm = vector<double>(
					prepcm.begin() + hop*(writeLength),
					prepcm.end()
				);
			}else{
				prepcm=vector<double>();
			}
			sound = postsound;
		}
	}
}
