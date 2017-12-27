#include <vector>
#include <string>
#include <iostream>

namespace fileio{
	/* This namespace just contains functions
	** commonly used to read from and write to files and filestreams.
	** TODO:change all to use iostreams, not filenames.
	*/

	//custom error for inability to read files
	class fileOpenError: public std::runtime_error{
		public:
			fileOpenError();
	};

	// writes raw wav data to filename
	// truncates if file exists
	// TODO:is this even used?
	void wavWrite(std::vector<float>sound,std::string filename);
	// reads raw wav data from path
	std::vector<float> wavRead(std::string path);
	// writes raw wav data to filename
	// appends if file exists
	void append(std::vector<float>sound, std::string filename);

	// write functions write type to ostream in binary form
	// to be read in later with read functions
	void write(std::ostream& outFile, int);
	void write(std::ostream& outFile, float);
	void write(std::ostream& outFile, std::vector<int>);
	void write(std::ostream& outFile, std::vector<float>);
	void write(std::ostream& outFile, std::vector<std::vector<float>>);

	// read functions read binary data in from istreams
	// and interpret it at a certain type,
	// which is given as a dummy second argument
	int                              read(std::istream& inFile, int);
	float                           read(std::istream& inFile, float);
	std::vector<int>              read(std::istream& inFile, std::vector<int>);
	std::vector<float>              read(std::istream& inFile, std::vector<float>);
	std::vector<std::vector<float>> read(std::istream& inFile, std::vector<std::vector<float>>);
}
