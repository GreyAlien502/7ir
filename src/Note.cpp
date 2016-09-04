#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>

#include "Note.h"

using namespace std;

/*
Note::Note(string lyric, int notenum, double delta, double duration, double length){
}

Note::Note(string lyric, int notenum, double delta, double duration, double length){
	map<string,string> parameterList = parameters(fileobject);
	
	lyric = parameterList["Lyric"];

	notenum = stoi(parameterList["NoteNum"]);
	velocity = stod(parameterList["Velocity"])/100.;
	delta = stod(parameterList["Delta"]);
	duration = stod(parameterList["Duration"]);
	length = stod(parameterList["Length"]);
}

Phone Note::getPhone(VoiceLibrary library){
	if(!library.hasPhone(lyric)){
		cerr<<lyric<<" not found.\n";
		return Phone();
	}
	return library.getPhone(lyric);
}

Sound Note::getSound(VoiceLibrary library){
	if(!library.hasPhone(lyric)){
		cerr<<lyric<<" not found.\n";
		return Sound(vector<double>(length,0));
	}
	Phone phone = library.getPhone(lyric);
	Sound output = phone.sample;
	output.transpose( 440.*pow(2.,(notenum-5-69.)/12.) / phone.frequency );
	output.setLength(phone.consonant, output.hops, length/1000*library.sampleRate/library.hop);
	for(int hop=0; hop<output.hops; hop++){
		for(int freq=0; freq<output.magnitudes[0].size(); freq++){
			output.magnitudes[hop][freq] *= velocity*.1;
		}
	}
	return output;
}
*/
