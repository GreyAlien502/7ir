#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "Song.h"
#include "fileio.h"

using namespace std;

map<string,string> parameters(ifstream& fileobject){
	map<string,string> output = map<string,string>();

	long lastPosition = fileobject.tellg();
	for(string line; getline(fileobject, line);){
		if(line[0] == '[' & line[line.length()-1] == ']'){
			fileobject.seekg(lastPosition);
			return output;
		}
		if(line[0] == '#'){cerr<<line;continue;}

		string::size_type equals = line.find('=');
		if(equals == string::npos){cerr<<"unused line in file.\n";continue;}
		output.insert({line.substr(0,equals), line.substr(equals+1)});
		lastPosition = fileobject.tellg();
	}
}

Song::Song(string path){
	ifstream ust(path);
	string line;
	if(ust.is_open()){
		int version = 1;
		while(line != "[#SETTING]"){
			if(line=="UST Version2.0"){ version = 2; }
			getline(ust, line);
		}
		map<string,string> parameterlecian = parameters(ust);
		
		tempo = stod(parameterlecian["Tempo"]);
		projectName = parameterlecian["ProjectName"];
		outFile = parameterlecian["OutFile"];
		voiceDir = parameterlecian["VoiceDir"];

		double delta = 0;
		int i = 0;
		getline(ust,line);
		while(line != "[#TRACKEND]"){
			map<string,string> parameterList = parameters(ust);
			
			string lyric = parameterList["Lyric"];
			double length = stod(parameterList["Length"])/1000.;
			double notenum;
			double velocity;
			double duration;
			if((version == 1) && (lyric == "R")){
				delta += length;
			}else{
				notenum = stoi(parameterList["NoteNum"]);

				if(version == 1){
					duration = length;
					velocity = 1;
				}else{
					velocity = stod(parameterList["Velocity"])/100.;
					delta = stod(parameterList["Delta"])/1000.;
					duration = stod(parameterList["Duration"])/1000.;
				}

				cerr<<lyric;
				notes.push_back(Note(
					lyric,
					notenum,
					velocity,
					delta,
					duration,
					length
				));

				if(version == 1){
					if((i != 0)){
						notes[i-1].length = delta;
					}
					delta = length;
					i++;
				}
			}
			getline(ust,line);
		}
		notes[notes.size()].length = notes[notes.size()].duration;
	}else{
		cerr<<"Couldn't open ust file\n";
		exit(1);
	}
}

void correlate(Sound& sound1, Sound& sound2, int start, int duration){
	int corlen = sound1.hops - start;
	/*
	for(int hopindex = 0; hopindex < corlen; hopindex++){
		double centroid =
			  sound1.getCentroid(start+hopindex) * double(hopindex)/corlen
			+ sound2.getCentroid(hopindex) * (1 - double(hopindex)/corlen);
		sound1.setCentroid(hopindex, centroid);
		//sound2.setCentroid(hopindex, centroid);
	}
	*/
}

void Song::synthesize(VoiceLibrary library, string filename){
	int sampleRate = library.sampleRate;
	
	if(notes.size()==1){
		cerr<<notes[0].lyric;
		fileio::write(
			library.getPhone(notes[0]).sample.synthesize(),
			filename
		);
	}else{
		vector<double> prepcm = vector<double>();
		Phone actuaphone = library.getPhone(notes[0]);
		Phone postphone;


		for(int i=0; i<notes.size(); i++){
			cerr<<notes[i].lyric;
			if(i<notes.size()-1){
				//Ni+1.sound
				postphone = library.getPhone(notes[i+1]);
				
				/*correlate Ni, Ni+1
				correlate(
					actuaphone
					postphone,
					postphane.delta*sampleRate/
						+actuaphone.preutter
						-postphone.preutter,
				);//*/
			}

			//Ni.synth
			vector<double> pcm = actuaphone.sample.synthesize();

			//add Ni-1, Ni
			if(prepcm.size() < pcm.size()){//expand prepcm if needed
				prepcm.resize(pcm.size(), 0);
			}

			for(int j=0; j<pcm.size(); j++){
				prepcm[j] += pcm[j];
			}


			//write part Ni-1,i
			int writeLength;
			if(i<notes.size()-1){
				writeLength = 
					notes[i+1].delta*sampleRate
					+ actuaphone.getPreutter()
					- postphone.getPreutter();
			}else{
				writeLength = 
					actuaphone.getPreutter();
					+ notes[i].duration*sampleRate;
			}
			if(prepcm.size() < writeLength){
				prepcm.resize(writeLength, 0);
			}
			fileio::append(
				vector<double>(
					prepcm.begin(),
					prepcm.begin() + writeLength
				),
				filename
			);

			//SHIFT
			if(writeLength<prepcm.size()){
				prepcm = vector<double>(
					prepcm.begin() + writeLength,
					prepcm.end()
				);
			}else{
				prepcm=vector<double>();
			}
			actuaphone = postphone;
		}
	}
}
