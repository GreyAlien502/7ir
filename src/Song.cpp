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
	map<string,string> output = map<string,string>();

	long lastPosition = fileobject.tellg();
	for(string line; getline(fileobject, line);){
		if((line[0] == '[' ) & (line[line.length()-1] == ']')){
			fileobject.seekg(lastPosition);
			return output;
		}
		if(line[0] == '#'){cerr<<line;continue;}

		string::size_type equals = line.find('=');
		if(equals == string::npos){cerr<<"unused line in file.\n";continue;}
		output.insert({line.substr(0,equals), line.substr(equals+1)});
		lastPosition = fileobject.tellg();
	}
	throw runtime_error("Invalid File");
}


double freqFromNum(int notenum){
	return 440.*pow(2.,(notenum-69.)/12.) ;
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
		
		tempo = stod(parameterlecian["Tempo"])/60.;
		projectName = parameterlecian["ProjectName"];
		outFile = parameterlecian["OutFile"];
		voiceDir = parameterlecian["VoiceDir"];

		double delta = 0;
		int i = 0;
		getline(ust,line);
		while(line != "[#TRACKEND]"){
			map<string,string> parameterList = parameters(ust);
			
			string lyric = parameterList["Lyric"];
			double length = stod(parameterList["Length"])/480.;
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
					delta = stod(parameterList["Delta"])/480.;
					duration = stod(parameterList["Duration"])/480.;
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
		notes[notes.size()-1].length = notes[notes.size()-1].duration;
	}else{
		cerr<<"Couldn't open ust file\n";
		exit(1);
	}
}


void Song::synthesize(VoiceLibrary library, string filename){
	int sampleRate = library.sampleRate;

	Phone phoneNow = library.getPhone(notes[0]);
	Speech speech = Speech(phoneNow.sample.startToSound(0).compatibleSound(
		vector<double>(phoneNow.overlap*sampleRate,0)
	));
	for(int note=0;note<notes.size();note++){
		cerr<<notes[note].lyric;
		double leftoverLength = speech.duration -phoneNow.overlap;
		Phone phoneNext;

		if(note+1<notes.size()){
			phoneNext = library.getPhone(notes[note+1]);
		//stretch next
		//cerr<<'e';
			if(phoneNext.preutter>notes[note].length/tempo){
				double newPreutter = notes[note].length/tempo;
				double newOverlap = phoneNext.overlap * newPreutter/phoneNext.preutter;
				phoneNext.sample.stretch(0,phoneNext.preutter,newPreutter);
				phoneNext.preutter = newPreutter;
				phoneNext.overlap = newOverlap;
			}
		}else{
			phoneNext = Phone();
			phoneNext.preutter=phoneNext.overlap=0;
		}
		//stretch now
		//cerr<<'o';
		double targetLength = notes[note].length/tempo
			-phoneNext.preutter
			+phoneNext.overlap;
		double vowelLength = min(
			notes[note].duration/tempo,
			targetLength
		);
		phoneNow.sample.stretch(
			phoneNow.preutter,
			phoneNow.sample.duration,
			vowelLength
		);
		//add space
		//cerr<<'s';
		double restLength = targetLength - vowelLength;
		if(restLength!=0){
			phoneNow.sample.add(
				Speech(phoneNow.sample.startToSound(0).compatibleSound(
					vector<double>(restLength*sampleRate,0)
				)),
				0
			);
		}

		//add to previous
		//cerr<<'a';
		speech.add(phoneNow.sample, phoneNow.overlap);
		//transpose
		//cerr<<'t';
		double noteBoundary = leftoverLength +phoneNow.preutter;
		double writeLength = noteBoundary +notes[note].length/tempo -phoneNext.preutter;
		double freq1,freq2;
		freq2 = freqFromNum(notes[note].notenum);
		if(note!=0){
			freq1 = freqFromNum(notes[note-1].notenum);
		}else{
			freq1 = freq2;
		}
		function<double (double)> frequency = [noteBoundary,freq1,freq2](double time){
			if(time<noteBoundary){
				return freq1;
			}else{
				return freq2;
			}
		};
		speech.transpose(frequency,writeLength);
		//pop&write
		//cerr<<'p';
		fileio::append(speech.pop(writeLength),filename);
		//reassign
		//cerr<<'r';
		phoneNow = phoneNext;
	}
}
