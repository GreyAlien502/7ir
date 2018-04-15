#include <string>
#include <functional> //function
#include <map>
#include <tuple> //tuple

class Note{
	/* This class contains all the note information stored for each note (e.g. in a UST file).
	** All this is independent of the voice library that will be used to produce it.
	*/
	private:
		std::map<std::string,std::string> parameters;
		bool validate();
		// get parameter values from parameter dict
		std::string getStringParam(std::string paramName, std::string defaultValue);
		float getFloatParam(std::string paramName, float scaleFactor, float defaultValue);
		std::tuple< std::vector<float>,std::vector<float> > getPitches();
	public:
		std::string lyric;
		int notenum; //MIDI note number
		float velocity;
		float delta;//in beats
		float duration;//in beats
		float length;//in beats

		std::vector<float> pitchTimes; // times of pitch in ms
		std::vector<float> pitches; // pitches in cents

		Note(std::map<std::string,std::string>);
		//Note(
		//	std::string lyric, int notenum, float velocity, float delta, float duration, float length,
		//	std::vector<float> pitchTimes=std::vector<float>(1), std::vector<float> pitches=std::vector<float>(1)
		//);

		std::function<float (float)> frequency(float previousFreq);
};
