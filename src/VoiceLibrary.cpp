#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <experimental/filesystem>
namespace filesystem=std::experimental::filesystem;

#include "fileio.h"
#include "VoiceLibrary.h"
#include "conversionTable.h"

using namespace std;

int getNoteNum(string noteName){
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

string VoiceLibrary::getFormatString(){
	uint16_t test = 0x0102;
	char* byte = reinterpret_cast<char*>(&test);
	string endianness = to_string(byte[0]) + to_string(byte[1]);
	char VERSION = '1';

	return
		to_string(sizeof(int))+','+
		to_string(sizeof(float))+','+
		endianness+','+
		to_string(hop)+','+
		to_string(windowLength)+','+
		to_string(sampleRate)+','+
		VERSION;
}

string convert(string inLyric){
	vector< pair<string,string> >::iterator outPair = find_if(
		begin(conversionTable),
		end(conversionTable),
		[&inLyric](pair<string,string> testPair){return testPair.first == inLyric;}
	);
	if(outPair != end(conversionTable)){
		return outPair->second;
	}
	outPair = find_if(
		conversionTable.begin(), 
		conversionTable.end(),
		[&inLyric](pair<string,string> testPair){return testPair.second == inLyric;}
	);
	if(outPair != conversionTable.end()){
		return outPair->first;
	}
	return inLyric;
}

string VoiceLibrary::affixedLyric(int noteNum, string lyric){
	if(prefixMap.size() == 0){
		return lyric;
	}

	string output;
	if(noteNum < minNoteNum){
		output = '-'+lyric;
		if(hasPhone(output)){
			return output;
		}
		output = "↓"+lyric;
		if(hasPhone(output)){
			return output;
		}
	}
	if(noteNum > maxNoteNum){
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

	return lyric;
}

VoiceLibrary::VoiceLibrary(std::string path, int windowOverlap, int windowSize, int rate){
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
	for(auto& currentFile : filesystem::directory_iterator(path)){
		string pathName = currentFile.path().native();
		if(filesystem::is_directory(currentFile.status())){
			if(compiling){ compile(currentFile.path().native());}
			importDir(currentFile.path().native());
		}
	}
	ofstream compiled_file(path+"/compilation");
	compiled_file << getFormatString();
}

bool endsIn(string searchee, string suffix){
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
				//remove old speech
				remove((path+".spch").c_str());
				//make new speech
				ofstream outputfile(path+".spch");
				Speech(Sound(
						fileio::wavRead(path),
						windowLength/hop,
						windowLength,
						sampleRate
				)).write(outputfile);
			}
		}
	}
}

void VoiceLibrary::importDir(string path){
	cerr<<endl<<path;
	vector<string> presets = vector<string> ();
	ifstream oto_ini(path+"/oto.ini");
	if(oto_ini.is_open()){
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

			start = end + 1;
			end = line.find(',', start);
			float offset = stod(line.substr(start,end-start))/1000.;

			start = end + 1;
			end = line.find(',', start);
			float consonant = stod(line.substr(start,end-start))/1000.;

			start = end + 1;
			end = line.find(',', start);
			float cutoff = stod(line.substr(start,end-start))/1000.;

			start = end + 1;
			end = line.find(',', start);
			float preutter = stod(line.substr(start,end-start))/1000.;

			start = end + 1;
			end = line.length();
			float overlap = stod(line.substr(start,end-start))/1000.;

			try{
				cerr<<alias;
				aliases.insert({alias,phones.size()});
				phones.push_back(tuple<string,float,float,float,float,float>(
					path+'/'+filename+".spch",
					offset,
					consonant,
					cutoff,
					preutter,
					overlap
				));
			}catch(fileio::fileOpenError& exc){
				cerr<<endl<<path+filename<<" not found."<<endl;
			}
		}
	}
}

bool VoiceLibrary::hasPhone(string alias){
	return aliases.find(alias) != aliases.end();
}

Phone VoiceLibrary::getPhone(Note note){
	tuple<string,float,float,float,float,float> phoneData;

	string lyric = affixedLyric(note.notenum,note.lyric);
	if(hasPhone(lyric)){
		phoneData = phones[aliases.at(lyric)];
	}

	lyric = affixedLyric(note.notenum,convert(note.lyric));
	if(hasPhone(lyric)){
		phoneData = phones[aliases.at(lyric)];
	}


	ifstream speechFile(get<0>(phoneData));
	if(speechFile.is_open()){
		Speech speechSample = Speech(speechFile);
		float offset = get<1>(phoneData);
		float consonant = get<2>(phoneData);
		float cutoff = get<3>(phoneData);
		float preutter = get<4>(phoneData);
		float overlap = get<5>(phoneData);

		if(cutoff >= 0){
			speechSample.crop(offset,speechSample.duration-cutoff);
		}else{
			speechSample.crop(offset,offset-cutoff);
		}

		return Phone(
			speechSample,
			consonant, preutter, overlap
		);
	}else{
		cerr<<get<0>(phoneData)+" not found"<<endl;
		return Phone(windowLength/hop,windowLength, sampleRate);
	}
}
