#include <vector>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <algorithm> 

#include "Song.h"
#include "fileio.h"

using namespace std;

int main(int args, char** argv){
	cerr << "loading song...";
	Song sang = Song(argv[1]);

	cerr <<endl<<"loading voice library...";
	VoiceLibrary library = VoiceLibrary(
		argv[2],
		16, //overlap
		1024 //windowSize
	);

	cerr <<endl<<"synthesizing...";
	sang.synthesize(library, "output.wav");
}
