#include <fstream>
#include <algorithm> // find
#include <math.h> // floor
#include <assert.h> // assert
#include <experimental/filesystem>
namespace filesystem=std::experimental::filesystem;

#include "fileio.h"
#include "VoiceLibrary.h"
#include "conversionTable.h"

using namespace std;

// gets note number from something like "C#4" or "B3"
static int getNoteNum(string noteName){
	map<char,int> letterMap = {
		{'C', 0},
		{'D', 2},
		{'E', 4},
		{'F', 5},
		{'G', 7},
		{'A', 9},
		{'B', 11}
	};

	int output = letterMap[noteName[0]];

	string octave;
	if(noteName[1] == '#'){
		output++;
		octave = noteName.substr(2);
	}else{
		octave = noteName.substr(1);
	}
	
	output += stoi(octave)*12;
	
	return output;
}

// returns a string that i
string VoiceLibrary::getFormatString(){
	uint16_t test = 0x0102;
	char* byte = reinterpret_cast<char*>(&test);
	string endianness = to_string(byte[0]) + to_string(byte[1]);
	string VERSION = "3";

	return VERSION;
}

//converts lyric from kana to RO-MAji and vice versa
static string convert(string inLyric){
	// try going one way
	vector< pair<string,string> >::iterator outPair = find_if(
		begin(conversionTable),
		end(conversionTable),
		[&inLyric](pair<string,string> testPair){return testPair.first == inLyric;}
	);
	if(outPair != end(conversionTable)){
		return outPair->second;
	}
	// try going the other way
	outPair = find_if(
		conversionTable.begin(), 
		conversionTable.end(),
		[&inLyric](pair<string,string> testPair){return testPair.second == inLyric;}
	);
	if(outPair != conversionTable.end()){
		return outPair->first;
	}
	return inLyric; // if all else fails, returrn original
}

string VoiceLibrary::affixedLyric(int noteNum, string lyric){
	if(prefixMap.size() == 0){
		return lyric;
	}

	string output;
	if(noteNum < this->minNoteNum){
		//try - and then ↓ as low prefices
		output = '-'+lyric;
		if(hasPhone(output)){
			return output;
		}
		output = "↓"+lyric;
		if(hasPhone(output)){
			return output;
		}
	}
	if(noteNum > this->maxNoteNum){
		//try * and then ↑ as high prefices
		output = '*'+lyric;
		if(hasPhone(output)){
			return output;
		}
		output = "↑"+lyric;
		if(hasPhone(output)){
			return output;
		}
	}

	if( prefixMap.find(noteNum) != prefixMap.end() ){
		 pair<string,string> affixes = prefixMap[noteNum];
		 return affixes.first + lyric + affixes.second;
	}

	return lyric; // wort case, return original
}

VoiceLibrary::VoiceLibrary(std::string path, int windowOverlap, int windowSize, float rate){
	//initialize class variables
	sampleRate = rate;
	windowLength = windowSize;
	hop = windowLength/windowOverlap;

	aliases = map<string,int>();

	//read in prefix.map
	ifstream prefix_map(path+"/prefix.map");
	if(prefix_map.is_open()){
		for(string line; getline(prefix_map, line);){
			int noteNameEnd = line.find('\t');
			int prefixEnd = line.find('\t',noteNameEnd+1);
			int noteNum = getNoteNum(line.substr(0,noteNameEnd));

			if(prefixMap.size() == 0){
				maxNoteNum = minNoteNum = noteNum;
			}else if(noteNum < minNoteNum){
				minNoteNum = noteNum;
			}else if(noteNum > maxNoteNum){
				maxNoteNum = noteNum;
			}

			prefixMap.insert({
				noteNum,
				{
					line.substr(noteNameEnd+1,prefixEnd-noteNameEnd-1),
					line.substr(prefixEnd+1),
				}
			});
		}
	}else{
		prefix_map = {};
	}

	//read in files based on oto.ini files
	ifstream compile_file(path+"/compilation");
	string formatString;
	getline(compile_file, formatString);
	bool compiling;
	if(!( compile_file.is_open() & (formatString == getFormatString()) )){
		compile_file.close();
		compiling = true;
		compile(path);
	}
	importDir(path);
	//for each file
	for(auto& currentFile : filesystem::directory_iterator(path)){
		if(filesystem::is_directory(currentFile.status())){
			if(compiling){ compile(currentFile.path().native());}
			importDir(currentFile.path().native());
		}
	}
	ofstream compiled_file(path+"/compilation");
	compiled_file << getFormatString();
}

//experimental frequency detection that hasn't been very accurate
/*float Speech::detectFrequency(vector<float> amplitudes,vector<float> frequencies){
	float minFreq = 82.407;//minimum frequency humans can sing
	int maxAmplitude = distance(
			amplitudes.begin(),
			max_element(amplitudes.begin()+minFreq*windowLength/sampleRate,amplitudes.end())
		);
	float maxFreq = frequencies[maxAmplitude];
	if(maxFreq<0){maxFreq=0;}
	int maxFreqNum =  maxFreq/minFreq;
	vector<float> correlations(maxFreqNum,1);
	for(int freqNum = 1; freqNum < maxFreqNum; freqNum +=1){
		float currentFreq = maxFreq/freqNum;
		for(int harmonic=0;harmonic<amplitudes.size()/maxFreq*sampleRate/windowLength;harmonic++){
			int currentIndex = currentFreq*harmonic/sampleRate*windowLength;
			correlations[freqNum-1] += amplitudes[currentIndex];
		}
	}
	float ferq = distance(
		correlations.begin(),
		max_element(correlations.begin(),correlations.end())
	)+1;
	if(maxFreq/ferq<250|maxFreq/ferq>350){
	}
	//cout<<maxFreq<<','<<ferq<<','<<maxFreqNum<<','<<maxFreq/ferq<<endl;
	return maxFreq/ferq;
}*/ //It's not very effective
// detects the fundamental frequency in an audio signal
static float detectFrequency(vector<float> pcm,float sampleRate){
	int length = pcm.size();
	int minPeriod = floor(sampleRate/1046.5);//maximum frequency humans can sing
	int maxPeriod = floor(sampleRate/82.407);//minimum frequency humans can sing
	vector<float> errors(maxPeriod-minPeriod,0);
	for(int period=minPeriod; period<maxPeriod; period++){
		int periods = length/period;//number of periods that fit in the sample
		for(int periodsIn=0; periodsIn<periods-1; periodsIn++){
			for(int t=0;t<period;t++){
				errors[period-minPeriod] += pow( pcm[period*(periodsIn) + t] * pcm[period*(periodsIn+1) + t],2);
			}
		}
		errors[period-minPeriod] /= (periods-1)*period;
	}
	int maximum = distance( errors.begin(), max_element(errors.begin(),errors.end()) );

	//try to catch octave errors TODO:make (more?) effective
	int fundamentalIndex = maximum;
	/*
	for(int index=maximum/2; index*0>minPeriod; index--){
		if( errors[index] > errors[maximum]*.75 ){
			fundamentalIndex = (2*index+1)/(2*maximum);
		}
	}
	*/
	return float(sampleRate)/(minPeriod+fundamentalIndex);
}
//returns if searchee ends in suffix
static bool endsIn(string searchee, string suffix){
	return searchee.size() >= suffix.size() &&
		searchee.compare(searchee.size()-suffix.size(), suffix.size(), suffix)==0;
}
void VoiceLibrary::compile(string path){
	ifstream oto_ini(path+"/oto.ini");
	if(oto_ini.is_open()){
		oto_ini.close();
		for(auto& currentFile : filesystem::directory_iterator(path)){
			string path = currentFile.path().native();
			if(endsIn(path,".wav")){
				cerr<<path<<endl;
				ofstream outputfile(path+".frq");
				ifstream audioFile(path);
				if(!audioFile.is_open()){throw(fileio::fileOpenError(path));}
				outputfile << detectFrequency(
					fileio::wavRead(audioFile),
					sampleRate
				);
			}
		}
	}
}

void VoiceLibrary::importDir(string path){
	vector<string> presets = vector<string> ();
	ifstream oto_ini(path+"/oto.ini");
	if(oto_ini.is_open()){
		cerr<<endl<<path;
		for(string line; getline(oto_ini, line);){
			if(line[0] == '#'){continue;}
			int start = 0;
			int end;

			end = line.find('=');
			string filename = line.substr(start,end-start);

			start = end + 1;
			end = line.find(',', start);
			string alias = line.substr(start,end-start);

			string settings = filename + line.substr(end+1, line.length());
			vector<string>::iterator repeat = find(presets.begin(), presets.end(), settings);
			if(repeat != presets.end()){
				aliases.insert({
					alias,
					distance(presets.begin(), repeat)
				});
				continue;
			}
			presets.push_back(settings);

			std::vector<float> timings(5);
			for( int timingNum=0; timingNum<timings.size(); timingNum++){
				start = end + 1;
				end = line.find(',', start);
				try{
					timings[timingNum] = stod(line.substr(start,end-start))/1000.;
				}catch(std::invalid_argument){
					cerr<<"OHNO";
					timings[timingNum] = 0;
				}
			}

			try{
				cerr<<alias;
				aliases.insert({alias,phones.size()});
				phones.push_back(tuple<string,float,float,float,float,float>(
					path+'/'+filename,
					timings[0],
					timings[1],
					timings[2],
					timings[3],
					timings[4]
				));
			}catch(fileio::fileOpenError& exc){
				cerr<<endl<<"file '"<<path+'/'+filename<<"' not found."<<endl;
			}
		}
	}
}

bool VoiceLibrary::hasPhone(string alias){
	return aliases.find(alias) != aliases.end();
}

Phone VoiceLibrary::getPhone(Note note){
	tuple<string,float,float,float,float,float> phoneData;

	// use the affixed lyric 
	string lyric = affixedLyric(note.notenum,note.lyric);
	if(hasPhone(lyric)){
		phoneData = phones[aliases.at(lyric)];
	}

	// or the converted version of the lyric
	lyric = affixedLyric(note.notenum,convert(note.lyric));
	if(hasPhone(lyric)){
		phoneData = phones[aliases.at(lyric)];
	}

	float offset = get<1>(phoneData);
	float consonant = get<2>(phoneData);
	float cutoff = get<3>(phoneData);
	float preutter = get<4>(phoneData);
	float overlap = get<5>(phoneData);

	// then read in from the file
	std::ifstream audioFile(get<0>(phoneData)+"");
	std::ifstream freqFile( get<0>(phoneData)+".frq");
	if( !audioFile.is_open() | !freqFile.is_open() ){
		cerr<<"Warning: could not load '"<<note.lyric<<"'";
		//if all else fails, just complain and return emtpy phone
		return Phone(windowLength/hop,windowLength, sampleRate);
	}
	int end;
	std::ios::seekdir direction;
	if(cutoff >= 0){
		direction = std::ios::end;
		end = cutoff*sampleRate;
		//sample.crop(offset,sample.duration-cutoff);
	}else{
		direction = std::ios::beg;
		end = (offset-cutoff)*sampleRate;
		//sample.crop(offset,offset-cutoff);
//		sample = std::vector<float>(
//			sample.begin()+offset/sampleRate,
//			sample.begin()+(offset-cutoff)/sampleRate
//		);
	}
	float frequency;
	freqFile >> frequency;

	Speech sample = Speech(
		Sound(fileio::wavRead(
			audioFile,
			offset*sampleRate,
			end,
			direction
		), this->windowLength/hop, this->windowLength, this->sampleRate)
		,frequency
	);

	return Phone(
		sample,
		consonant, preutter, overlap
	);
}
