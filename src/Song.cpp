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

static map<string,string> parameters(ifstream& fileobject){
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
static float freqFromNum(int notenum){
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
		for( getline(ust,line); line != "[#TRACKEND]"; getline(ust,line) ){
			// get note parameters for each note
			Note currentNote = Note(parameters(ust));
			
			if(version == 1){
				// duration is implied in v1
				currentNote.duration = currentNote.length;
				if(currentNote.lyric == "R"){
					// "R" means rest in v1.
					delta += currentNote.length;
					continue;
				}
				if(i != 0){
					notes[i-1].length = delta;
				}
				delta = currentNote.length;
				i++;
			}

			notes.push_back(currentNote);
			cerr<<currentNote.lyric;//log each lyric in the song

		}
		notes[notes.size()-1].length = notes[notes.size()-1].duration;
	}else{
		cerr<<"Couldn't open ust file\n";
		exit(1);
	}
}

Phone Song::getNewPhone(int noteIndex,VoiceLibrary& library){
	if(noteIndex>=notes.size()){
		Phone output = library.getPhone(Note(map<string,string>()));
		return output;
	}
	Phone output = library.getPhone(notes[noteIndex]);
	if(output.preutter>notes[noteIndex-1].length/tempo){
		//stretch to fit preutten within previous note
		float newPreutter = notes[noteIndex-1].length/tempo;
		float newOverlap = output.overlap * newPreutter/output.preutter;
		output.sample.stretch(0,output.preutter,newPreutter);
		output.preutter = newPreutter;
		output.overlap = newOverlap;
	}
	return output;
}
void Song::resizePhone(Phone& resizee, int noteIndex, Phone& phoneNext, float sampleRate){
	//stretch to proper length
	float targetLength = notes[noteIndex].length/tempo
		-phoneNext.preutter
		+phoneNext.overlap;
	float vowelLength = min(
		notes[noteIndex].duration/tempo,
		targetLength
	);
	resizee.sample.stretch(
		resizee.preutter,
		resizee.sample.duration,
		vowelLength
	);
	//add silence after notes if needed
	float restLength = targetLength - vowelLength;
	if(restLength!=0){
		resizee.sample.add(
			Speech(resizee.sample.startToSound(0).compatibleSound(
				vector<float>(restLength*sampleRate,0)
			)),
			0
		);
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

		Phone phoneNext = this->getNewPhone(note+1,library);

		resizePhone(phoneNow, note, phoneNext, sampleRate);

		//add modified note to previous speech sample
		speech.add(phoneNow.sample, phoneNow.overlap);

		//transpose
		float noteBoundary = leftoverLength +phoneNow.getPreeffectLength();
		float writeLength = noteBoundary +notes[note].length/tempo -phoneNext.getPreeffectLength();
		float freq1,freq2;
		freq2 = freqFromNum(notes[note].notenum);
		if(note!=0){
			freq1 = freqFromNum(notes[note-1].notenum);
		}else{
			freq1 = freq2;
		}
		function<float(float)> newFrequency = notes[note].frequency(freq1);
		speech.transpose(
			[noteBoundary,newFrequency](float time){
				return newFrequency(time-noteBoundary);
			},
			writeLength
		);
		//pop&write
		fileio::append(speech.pop(writeLength),file);
		//reassign
		phoneNow = phoneNext;
	}

	fileio::updateWavHeader(file);
}
