#include <vector>
#include <string>
#include <iostream>
#include <stdexcept> //runtime_error

namespace fileio{
	/* This namespace just contains functions
	** commonly used to read from and write to files and filestreams.
	*/

	//custom error for inability to read files
	class fileOpenError: public std::runtime_error{
		public :
			fileOpenError(std::string filename):
				runtime_error("Couldn't open '"+filename+"'."){}
	};

	// reads raw wav data from path
	std::vector<float> wavRead(std::ifstream& file, int startSample=0, int endSample=0,std::ios::seekdir direction=std::ios::end);
	// writes raw wav data to filename
	// appends if file exists
	void writeWavHeader(int sampleRate, std::ofstream& file);
	void updateWavHeader(std::ofstream& file);
	void append(std::vector<float>sound, std::ofstream& file);
}
