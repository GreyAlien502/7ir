#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <functional>
#include <algorithm>

#include "Song.h"
#include "fileio.h"

using namespace std;

map<string,string> parameters(ifstream& fileobject){
	/* reads through ifstream formatted as:
	**	[header 1]
	**	key=value
	**	...
	**	key=value
	**	[header 2]
	** saves the key value pairs into a map until it reaches a "[header]" line
	** This is how information is stored in USTs.
	*/
	map<string,string> output = map<string,string>();

	long lastPosition = fileobject.tellg();
	for(string line; getline(fileobject, line);){
		if((line[0] == '[' ) & (line[line.length()-1] == ']')){
			fileobject.seekg(lastPosition); // go back; you went too far.
			return output;
		}
		if(line[0] == '#'){cerr<<line;continue;} //ignore comments

		string::size_type equals = line.find('=');
		if(equals == string::npos){
			cerr<<"unused line in file.\n";continue;
		}// say something if there's a line without '=', but continue

		output.insert({
			line.substr(0,equals),
			line.substr(equals+1)
		});
		lastPosition = fileobject.tellg();
	}
	// If you went through the whole file,
	// something is wrong.
	// The file should end with "[#TRACKEND]".
	throw runtime_error("Invalid File");
}

// turns MIDI note number to the associated frequency in Hz.
float freqFromNum(int notenum){
	return 440.*pow(2.,(notenum-69.)/12.) ;
}


Song::Song(string path){
	ifstream ust(path);
	string line;
	if(ust.is_open()){
		int version = 1; //default version to 1
		while(line != "[#SETTING]"){//before any settings is optional version number
			if(line=="UST Version2.0"){ version = 2; }
			getline(ust, line);
		}
		
		//get initial setting parameters
		map<string,string> parameterlecian = parameters(ust);
		tempo = stod(parameterlecian["Tempo"])/60.; //convert bpm to Hz
		projectName = parameterlecian["ProjectName"];
		outFile = parameterlecian["OutFile"];
		voiceDir = parameterlecian["VoiceDir"];

		float delta = 0;
		int i = 0;
		getline(ust,line);
		while(line != "[#TRACKEND]"){
			// get note parameters for each note
			map<string,string> parameterList = parameters(ust);
			
			string lyric = parameterList["Lyric"];
			float length = stod(parameterList["Length"])/480.;// convert weird units to beats
			float notenum;
			float velocity;
			float duration;
			if((version == 1) && (lyric == "R")){ // "R" means rest in v1.
				delta += length;
			}else{
				notenum = stoi(parameterList["NoteNum"]);

				if(version == 1){
					duration = length;
					velocity = 1;
				}else{
					velocity = stod(parameterList["Velocity"])/100.; //normalized to (0,1)
					delta = stod(parameterList["Delta"])/480.;// convert to s
					duration = stod(parameterList["Duration"])/480.;// convert to s
					//TODO:change 480 to constant so i can change it.
				}

				cerr<<lyric;//log each lyric in the song
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
		notes[notes.size()-1].length = notes[notes.size()-1].duration;
	}else{
		cerr<<"Couldn't open ust file\n";
		exit(1);
	}
}


void Song::synthesize(VoiceLibrary library, ofstream& file){
	int sampleRate = library.sampleRate;
	fileio::writeWavHeader( sampleRate, file);

	Phone phoneNow = library.getPhone(notes[0]);
	Speech speech = Speech(phoneNow.sample.startToSound(0).compatibleSound(
		vector<float>(phoneNow.overlap*sampleRate,0)
	));//make empty speech to fill.
	for(int note=0;note<notes.size();note++){
		cerr<<notes[note].lyric; //log each lyric
		float leftoverLength = speech.duration -phoneNow.overlap;
		Phone phoneNext;

		if(note+1<notes.size()){
			phoneNext = library.getPhone(notes[note+1]);
		//stretch next note
			if(phoneNext.preutter>notes[note].length/tempo){
				float newPreutter = notes[note].length/tempo;
				float newOverlap = phoneNext.overlap * newPreutter/phoneNext.preutter;
				phoneNext.sample.stretch(0,phoneNext.preutter,newPreutter);
				phoneNext.preutter = newPreutter;
				phoneNext.overlap = newOverlap;
			}
		}else{
			phoneNext = Phone();
			phoneNext.preutter=phoneNext.overlap=0;
		}
		//stretch current note
		float targetLength = notes[note].length/tempo
			-phoneNext.preutter
			+phoneNext.overlap;
		float vowelLength = min(
			notes[note].duration/tempo,
			targetLength
		);
		phoneNow.sample.stretch(
			phoneNow.preutter,
			phoneNow.sample.duration,
			vowelLength
		);
		//add silence between notes if needed
		float restLength = targetLength - vowelLength;
		if(restLength!=0){
			phoneNow.sample.add(
				Speech(phoneNow.sample.startToSound(0).compatibleSound(
					vector<float>(restLength*sampleRate,0)
				)),
				0
			);
		}

		//add modified note to previous speech sample
		speech.add(phoneNow.sample, phoneNow.overlap);
		//transpose
		float noteBoundary = leftoverLength +phoneNow.preutter;
		float writeLength = noteBoundary +notes[note].length/tempo -phoneNext.preutter;
		float freq1,freq2;
		freq2 = freqFromNum(notes[note].notenum);
		if(note!=0){
			freq1 = freqFromNum(notes[note-1].notenum);
		}else{
			freq1 = freq2;
		}
		function<float (float)> frequency = [noteBoundary,freq1,freq2](float time){
			if(time<noteBoundary){
				return freq1;
			}else{
				return freq2;
			}
		};
		speech.transpose(frequency,writeLength);
		//pop&write
		fileio::append(speech.pop(writeLength),file);
		//reassign
		phoneNow = phoneNext;
	}

	fileio::updateWavHeader(file);
}
