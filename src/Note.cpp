#include <string> // string
#include <functional> // function
#include <map> // map
#include <assert.h> //assert
#include <iostream> //cerr
#include <fstream> //ofstream
#include <math.h> //pow
#include <vector> //vector
#include <functional> //function

#include "Note.h"
//using string = std::string;

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

bool Note::validate(){
	assert(this->pitchTimes.size() == this->pitches.size());
	return true;
}

template <typename T>
static T getParam(std::map<std::string,std::string>& noteParams, const std::string& paramName, T& defaultValue, std::function<T (std::string)> processString ){ //TODO: pass as const string& ?
	T value;
	try{
		value = processString(noteParams[paramName]);
	}catch(std::invalid_argument&){
		value = defaultValue;
	}
	return value;
	
}

float Note::getFloatParam(string paramName, float scaleFactor, float defaultValue=0){
	return getParam(
		this->parameters,paramName,defaultValue,
		(std::function<float (std::string)>) [scaleFactor](std::string stringValue)-> float { return stod(stringValue)/scaleFactor; }
	);
}
std::string Note::getStringParam(string paramName, string defaultValue){
	std::function<std::string(std::string)> process = [](std::string value) -> std::string { return value; };
	return getParam(
		this->parameters, paramName, defaultValue, process
	);
}


Note::Note(std::map<std::string,std::string> parameters){
	this->parameters = parameters;
		// TODO:techincially some of these should default to the next note's length...
		// but it should always be there(and valid)
	const float ticksPerBeat = 480.;
	const float defaultNoteNum = 24;

	// required entries
	this->lyric = getStringParam("Lyric","R");
	this->notenum = getFloatParam("NoteNum",1,defaultNoteNum); // convent from fleat to int for simlpicity
	this->length = getFloatParam("Length",ticksPerBeat);
	this->duration = getFloatParam("Duration",ticksPerBeat);
	this->delta = getFloatParam("Delta",ticksPerBeat);
	// TODO: support float preUtterance;

	// optional entries
	this->velocity = getFloatParam("Velocity",200,.5);


	assert(this->validate());
}
