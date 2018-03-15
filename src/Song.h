#include <vector>
#include <string>
#include <iostream>

#include "VoiceLibrary.h"


class Song{
	/* The Song object stores all the information about a Song,
	** independent of the voice library that will be used to synthesize it.
	** It stores all the information extracted from a UST file.
	** It is roughly equivalent to sheet music.
	*/
	private:
		/* Get new phone for the <noteIndex>th note.
		** Make sure its preutter isn't too big.
		** If there is no such note (out of range), return empty note.
		*/
		Phone getNewPhone(int noteIndex,VoiceLibrary& library);
		/* Resize <stretchee> to the appropriate length
		** for the <noteIndex>th note,
		** given <nextPhone> as the next Phone.
		** If there is space between <stretchee> and <nextPhone>,
		** add the appropriate amount of silence.
		*/
		void resizePhone(Phone& stretchee, int noteIndex, Phone& nextPhone, float sampleRate);//float space);

	public:
		float tempo; //in BPM
		std::string projectName; // name of project
		std::string outFile; // default output file path
		std::string voiceDir; // default voice library location
		std::vector<Note> notes; // list of Notes that make up the song

		Song(std::string path); // read Song in from UST at path
		//std::vector<float> synthesize(voiceLibrary::VoiceLibrary library); TODO: Is this even useful?
		void synthesize(VoiceLibrary, std::ofstream& file ); // synthesize song using a voicelibrary
		                                                 // and write output to path
};
