#include <string>

class Note{
	/* This class contains all the note information stored for each note (e.g. in a UST file).
	** All this is independent of the voice library that will be used to produce it.
	*/
	public:
		std::string lyric;
		int notenum; //MIDI note number
		float velocity;
		float delta;//in beats
		float duration;//in beats
		float length;//in beats

		Note(std::string lyric, int notenum, float velocity, float delta, float duration, float length):
			lyric(lyric),
			notenum(notenum),
			velocity(velocity),
			delta(delta),
			duration(duration),
			length(length){}
};
