#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>

#include "Song.h"

using namespace std;

map<string,string> song::parameters(ifstream& fileobject){
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

song::Note::Note(ifstream& fileobject){
	map<string,string> parameterList = song::parameters(fileobject);
	
	lyric = parameterList["Lyric"];

	notenum = stoi(parameterList["NoteNum"]);
	velocity = stod(parameterList["Velocity"]);
	delta = stod(parameterList["Delta"]);
	duration = stod(parameterList["Duration"]);
	length = stod(parameterList["Length"]);
}

voiceLibrary::Phone song::Note::getPhone(voiceLibrary::VoiceLibrary library){
	if(!library.hasPhone(lyric)){
		cerr<<lyric<<" not found.\n";
		return voiceLibrary::Phone();
	}
	return library.getPhone(lyric);
}

sound::Sound song::Note::getSound(voiceLibrary::VoiceLibrary library){
	if(!library.hasPhone(lyric)){
		cerr<<lyric<<" not found.\n";
		return sound::Sound(vector<double>(length,0));
	}
	voiceLibrary::Phone phone = library.getPhone(lyric);
	sound::Sound output = phone.sample;
	output.transpose( 440.*pow(2.,(notenum-69.)/12.) / phone.frequency );
	output.setLength(phone.consonant, output.hops, length/1000*library.sampleRate/library.hop);
	for(int hop=0; hop<output.hops; hop++){
		for(int freq=0; freq<output.magnitudes[0].size(); freq++){
			output.magnitudes[hop][freq] *= velocity;
		}
	}
	return output;
}
