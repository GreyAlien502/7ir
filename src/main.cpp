#include <vector>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <algorithm> 

#include "Song.h"
#include "fileio.h"

using namespace std;

int main(int args, char** argv){
	if( (args!=2) & (args!=4) ){
		cerr << "usage: re8 <ust-filename> <voicelibrary-filename> <output-filename>" << endl;
		exit(EXIT_FAILURE);
	}

	const int OVERLAP = 16;
	const int WINDOW_LENGTH = 1024;

	cerr << "loading song...";
	Song sang = Song(argv[1]);
	cerr <<endl<<"loading voice library...";

	if(args==4){
		sang.voiceDir = argv[2];
		sang.outFile = argv[3];
	}

	cerr <<endl<<"loading voice library...";
	VoiceLibrary library = VoiceLibrary(
		sang.voiceDir,
		OVERLAP, //overlap
		WINDOW_LENGTH //windowSize
	);


	cerr <<endl<<"synthesizing...";
	sang.synthesize(library, sang.outFile);
	cerr<<endl;

}
