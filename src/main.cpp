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
	Song sang = Song("ust.ust");
	cerr << "done.\n";

	cerr << "loading voice library...";
	VoiceLibrary library = VoiceLibrary(
		"voicelibrary",
		16, //overlap
		1024 //windowSize
	);
	cerr << "...done.\n";

	cerr << "synthesizing...";
	sang.synthesize(library, "output.wav");
	cerr << "done.\n";
}
