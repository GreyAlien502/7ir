#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <algorithm>

#include <boost/filesystem.hpp>

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

string getFormatString(){
	uint16_t test = 0x0102;
	char* byte = reinterpret_cast<char*>(&test);
	string endianness = to_string(byte[0]) + to_string(byte[1]);

	return
		to_string(sizeof(int))+','+
		to_string(sizeof(double))+','+
		endianness+','
		+'1';
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

	phones = vector<string>();
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
	bool compile;
	if(compile_file.is_open() & (formatString == getFormatString()) ){
		compile = false;
	}else{
		compile = true;
	}
	importDir(path,compile);
	for(boost::filesystem::directory_iterator currentFile(path); currentFile != boost::filesystem::directory_iterator(); currentFile++){
		if(boost::filesystem::is_directory(currentFile->status())){
			importDir(currentFile->path().native(), compile);
		}
	}
	ofstream compiled_file(path+"/compilation");
	compiled_file << getFormatString();
}

void VoiceLibrary::importDir(string path, bool compile){
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
			double offset = stod(line.substr(start,end-start))/1000.;

			start = end + 1;
			end = line.find(',', start);
			double consonant = stod(line.substr(start,end-start))/1000.;

			start = end + 1;
			end = line.find(',', start);
			double cutoff = stod(line.substr(start,end-start))/1000.;

			start = end + 1;
			end = line.find(',', start);
			double preutter = stod(line.substr(start,end-start))/1000.;

			start = end + 1;
			end = line.length();
			double overlap = stod(line.substr(start,end-start))/1000.;

			try{
				string phonePath = path+'/'+alias+".phone";
				if(compile){
					vector<double> pcm = fileio::wavRead(path+'/'+filename);
					if(cutoff > 0){
						pcm = vector<double>(
								pcm.begin()+offset*sampleRate,
								pcm.end()-cutoff*sampleRate);
					}else{
						pcm = vector<double>(
							pcm.begin()+offset*sampleRate,
							pcm.begin()+(offset-cutoff)*sampleRate);
					}
					ofstream phoneFile(phonePath, ios::binary);
					Phone(
						pcm,
						consonant, preutter, overlap,
						windowLength/hop, windowLength, sampleRate
					).write(phoneFile);
				}
				aliases.insert({alias,phones.size()});
				phones.push_back(phonePath);
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
	string lyric = affixedLyric(note.notenum,note.lyric);
	if(hasPhone(lyric)){
		ifstream phoneFile(phones[aliases.at(lyric)]);
		if(phoneFile.is_open()){
			return Phone(phoneFile);
		}
	}
	lyric = affixedLyric(note.notenum,convert(note.lyric));
	if(hasPhone(lyric)){
		ifstream phoneFile(phones[aliases.at(lyric)]);
		if(phoneFile.is_open()){
			return Phone(phoneFile);
		}
	}
	return Phone(windowLength/hop,windowLength, sampleRate);
}
