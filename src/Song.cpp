#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "Song.h"
#include "fileio.h"

using namespace std;

Song::Song(string path){
	ifstream ust(path);
	string line;
	if(ust.is_open()){
		while(line != "[#SETTING]"){
			getline(ust, line);
			cerr<<line<<endl;
		}
		map<string,string> parameterlecian = parameters(ust);
		
		tempo = stod(parameterlecian["Tempo"]);
		projectName = parameterlecian["ProjectName"];
		outFile = parameterlecian["OutFile"];
		voiceDir = parameterlecian["VoiceDir"];

		getline(ust,line);
		while(line != "[#TRACKEND]"){
			notes.push_back(Note(ust));
			getline(ust,line);
		}
	}else{
		cerr<<"Couldn't open ust file\n";
		exit(1);
	}
}

void correlate(Sound& sound1, Sound& sound2, int start, int duration){
	int corlen = sound1.hops - start;
	for(int hopindex = 0; hopindex < corlen; hopindex++){
		double centroid =
			  sound1.getCentroid(start+hopindex) * double(hopindex)/corlen
			+ sound2.getCentroid(hopindex) * (1 - double(hopindex)/corlen);
		sound1.setCentroid(hopindex, centroid);
		//sound2.setCentroid(hopindex, centroid);
	}
}

void Song::synthesize(VoiceLibrary library, string filename){
	int sampleRate = library.sampleRate;
	int hop = library.hop;
	
	if(notes.size()==1){
		cerr<<notes[0].lyric<<endl;
		fileio::write(
			library.getPhone(notes[0]).sample.synthesize(),
			filename
		);
	}else{
		vector<double> prepcm = vector<double>();
		Phone actuaphone = library.getPhone(notes[0]);
		Phone postphone;


		for(int i=0; i<notes.size(); i++){
			cerr<<notes[i].lyric;
			if(i<notes.size()-1){
				//Ni+1.sound
				postphone = library.getPhone(notes[i+1]);
				
				/*correlate Ni, Ni+1
				correlate(
					actuaphone
					postphone,
					postphane.delta/1000.*sampleRate/hop
						+actuaphone.preutter
						-postphone.preutter,
				);//*/
			}

			//Ni.synth
			vector<double> pcm = actuaphone.sample.synthesize();

			//add Ni-1, Ni
			if(prepcm.size() < pcm.size()){//expand prepcm if needed
				prepcm.resize(pcm.size(), 0);
			}

			for(int j=0; j<pcm.size(); j++){
				prepcm[j] += pcm[j];
			}


			//write part Ni-1,i
			int writeLength;
			if(i<notes.size()-1){
				writeLength = 
					actuaphone.preutter
					+ notes[i+1].delta/1000.*sampleRate/hop
					- postphone.preutter;
			}else{
				writeLength = 
					actuaphone.preutter
					+ notes[i].duration/1000.*sampleRate/hop;
			}
			fileio::append(
				vector<double>(
					prepcm.begin(),
					prepcm.begin() + hop*(writeLength)
				),
				filename
			);

			//SHIFT
			if(hop*(writeLength)<prepcm.size()){
				prepcm = vector<double>(
					prepcm.begin() + hop*(writeLength),
					prepcm.end()
				);
			}else{
				prepcm=vector<double>();
			}
			actuaphone = postphone;
		}
	}
}
