#include <map>
#include <string>

class Note{
	public:
		std::string lyric;
		int notenum;
		double velocity;
		double delta;//in ms
		double duration;//in ms
		double length;//in ms

		Note(std::string lyric, int notenum, double velocity, double delta, double duration, double length):
			lyric(lyric),
			notenum(notenum),
			velocity(velocity),
			delta(delta),
			duration(duration),
			length(length){}
};
