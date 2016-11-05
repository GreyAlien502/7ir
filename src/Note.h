#include <map>
#include <string>

class Note{
	public:
		std::string lyric;
		int notenum;
		double velocity;
		double delta;//in beats
		double duration;//in beats
		double length;//in beats

		Note(std::string lyric, int notenum, double velocity, double delta, double duration, double length):
			lyric(lyric),
			notenum(notenum),
			velocity(velocity),
			delta(delta),
			duration(duration),
			length(length){}
};
