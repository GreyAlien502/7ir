#include <string> // string
#include <functional> // function
#include <map> // map
#include <assert.h> //assert
#include <iostream> //cerr
#include <fstream> //ofstream
#include <math.h> //pow
#include <vector> //vector
#include <sstream> //stringstream

#include "Note.h"
//using string = std::string;

using namespace std;

static const float TICKS_PER_BEAT = 480.;
// turns MIDI note number to the associated frequency in Hz.
static float freqFromNum(int notenum){
	return 440.*pow(2.,(notenum-69.)/12.) ;
}

bool Note::validate(){
	assert(this->pitchTimes.size() == this->pitches.size());
	return true;
}

/*
 * Parse a row of comma separated values into a vector.
 * There is no escape.
*/
static vector<string> parseCSV(std::string input, char separator=',' ){
	stringstream inputStream(input);//add a separator to make sure an empty last element is not ignored
	vector<string> output(0);
	string field;
	while( getline(inputStream,field,separator) ){
		output.push_back(field);
	}
	return output;
}

template <typename T>
static T getParam(std::map<std::string,std::string>& noteParams, const std::string& paramName, T& defaultValue, std::function<T (std::string)> processString ){ //TODO: pass as const string& ?
	auto value = noteParams.find(paramName);
	if(value != noteParams.end()){
		try{
			return processString(value->second);
		}catch(std::invalid_argument&){}
	}
	return defaultValue;
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
tuple<vector<float>,vector<float>> Note::getPitches(){
	vector<float> pitchTimes(0);
	vector<float> pitches(0);

	vector<string> start = parseCSV( getStringParam("PBS","0"), ';' );

	pitchTimes.push_back(stod(start[0])/1000);// convert to seconds
	if(start.size()==2){
		pitches.push_back(stod(start[1])/10);// convert to cents
	}else{
		pitches.push_back(0);
	}

	vector<string> pbw = parseCSV( getStringParam("PBW",""), ',' );
	vector<string> pby = parseCSV( getStringParam("PBW",""), ',' );
	pby.resize(pbw.size(),"0"); // make sure they are the same size
	for(int i=0; i<pbw.size(); i++){
		pitchTimes.push_back( pitchTimes.back() + stod(pbw[i])/1000 ); // step time by pbw value
		pitches   .push_back( pitches   .back() + stod(pby[i])/10   );
	}
	return { pitchTimes, pitches };
}
Note::Note(std::map<std::string,std::string> parameters){
	this->parameters = parameters;
		// TODO:techincially some of these should default to the next note's length...
		// but it should always be there(and valid)
	const float defaultNoteNum = 24;

	// required entries
	this->lyric = getStringParam("Lyric","R");
	this->notenum = getFloatParam("NoteNum",1,defaultNoteNum); // convent from fleat to int for simlpicity
	this->length = getFloatParam("Length",TICKS_PER_BEAT);
	this->duration = getFloatParam("Duration",TICKS_PER_BEAT);
	this->delta = getFloatParam("Delta",TICKS_PER_BEAT);
	// TODO: support float preUtterance;

	// optional entries
	this->velocity = getFloatParam("Velocity",200,.5);
	std::tie(this->pitchTimes, this->pitches) = getPitches();


	assert(this->validate());
}

function<float (float)> Note::frequency(float previousFreq){
	float currentFreq = freqFromNum(this->notenum);
	return [previousFreq,currentFreq] ( float time ) -> float {
		if (time<0){
			return previousFreq;
		}else{
			return currentFreq;
		}
	};
}

