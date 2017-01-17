#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "fileio.h"
#include "VoiceLibrary.h"
#include "conversionTable.h"

using namespace std;

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


VoiceLibrary::VoiceLibrary(std::string path, int windowOverlap, int windowSize, int rate){
	sampleRate = rate;
	windowLength = windowSize;
	hop = windowLength/windowOverlap;

	phones = vector<basePhone>();
	aliases = map<string,int>();
	vector<string> presets = vector<string> ();

	ifstream oto_ini(path+"/oto.ini");
	if(oto_ini.is_open()){
		int i = 0;
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
			cerr<<"\t"<<alias;
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
				vector<double> pcm = fileio::read(path+'/'+filename);
				pcm = vector<double>(
							pcm.begin()+offset*sampleRate,
							pcm.end()-cutoff*sampleRate);

				phones.push_back(basePhone(
					pcm,
					consonant, preutter, overlap,
					windowOverlap, windowSize, sampleRate
				));
				aliases.insert({alias,i});
				i++;
			}catch(fileio::fileOpenError& exc){
				cerr<<endl<<filename<<" not found."<<endl;
			}
		}
	}else{
		cerr<<"Couldn't open oto.ini file\n";
		exit(1);
	}		
}

bool VoiceLibrary::hasPhone(string alias){
	return aliases.find(alias) != aliases.end();
}

Phone VoiceLibrary::getPhone(Note note, double tempo){
	string lyric = note.lyric;
	if(hasPhone(note.lyric)){
		return phones[aliases.at(lyric)].adjustPhone(note, tempo);
	}
	lyric = convert(note.lyric);
	if(hasPhone(lyric)){
		return phones[aliases.at(lyric)].adjustPhone(note, tempo);
	}
	return Phone();
}
