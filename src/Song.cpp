#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <functional>

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
	
	if(notes.size()==1){
		cerr<<notes[0].lyric;
		fileio::wavWrite(
			library.getPhone(notes[0]).sample.synthesize(),
			filename
		);
	}else{
		Speech speechStream = Speech();

		Phone actuaphone = library.getPhone(notes[0]);
		Phone postphone;


		double writeLength = 0;
		for(int i=0; i<notes.size(); i++){
			//stretch
			//cerr<<'s'<<endl;

			double vowelLength = notes[i].length/tempo;
			if ( i+1 < notes.size() ){
				postphone = library.getPhone(notes[i+1]);
				if(notes[i].length/tempo < postphone.preutter){
					//cerr<<"CEALLYsMOL\n";
					postphone.sample.stretch(
						0,
						postphone.preutter,
						notes[i].length/tempo
					);
					postphone.overlap *= notes[i].length/tempo/postphone.preutter;
					postphone.preutter = notes[i].length/tempo;
				}

				vowelLength +=
					-postphone.preutter
					+postphone.overlap;
			}
			actuaphone.sample.stretch(
				actuaphone.preutter,
				actuaphone.sample.duration,
				vowelLength
			);

			//combine
			//cerr<<'c'<<endl;
			if(i != 0){
				//TODO:fix error
				speechStream.add(actuaphone.sample,actuaphone.overlap);
			}else{
				speechStream = actuaphone.sample;
			}

			//transpose
			writeLength += notes[i].length/tempo + actuaphone.preutter;
			if( i+1 < notes.size() ){
				writeLength -= postphone.preutter;
			}


			//cerr<<'t'<<endl;
			double frequency = freqFromNum(notes[i].notenum);
			function<double (double)> frequencies = 
				[frequency](double time)->double{(void)time; return frequency;};
			speechStream.transpose(
				frequencies,
				writeLength
			);
			
			//pop&write
			//cerr<<'p'<<endl;
			vector<double> pcm = speechStream.pop(writeLength);
			writeLength -= double(pcm.size())/sampleRate;
			fileio::append(
				pcm,
				filename
			);
			actuaphone = postphone;
			cerr<<notes[i].lyric;
		}
	}
}
