#include <cmath>
#include <fstream>

#include "Song.h"
#include "fileio.h"

using namespace std;

int main(int args, char** argv){
	if( args!=4 ){ // TODO: allow for only UST if it contains default voicelibrary and output files
		cerr << "usage: re8 <ust-filename> <voicelibrary-filename> <output-filename>" << endl;
		exit(EXIT_FAILURE);
	}


	const int OVERLAP = 8; //<- these can be adjusted
	const int WINDOW_LENGTH = pow(2,9); // <-/

	cerr << "loading song...";
	Song sang = Song(argv[1]);

	if(args==4){//if we have all the arguments
		sang.voiceDir = argv[2];
		sang.outFile = argv[3];
	}// TODO:check the song for default voicelibrary and output files if not given these parameters
	std::ofstream outFile(sang.outFile, ios::out|ios::binary|ios::trunc);
	if(!outFile.is_open()){
		throw( fileio::fileOpenError() );
	}

	cerr <<endl<<"loading voice library...";
	VoiceLibrary library = VoiceLibrary(
		sang.voiceDir,
		OVERLAP,
		WINDOW_LENGTH
	);


	cerr <<endl<<"synthesizing...";
	sang.synthesize(library, outFile);
	cerr<<endl;

}
