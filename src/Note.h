#include <map>
#include <string>

class Note{
	public:
		std::string lyric;
		int notenum;
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
