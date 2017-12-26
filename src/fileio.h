#include <vector>
#include <string>
#include <iostream>

namespace fileio{
	class fileOpenError: public std::runtime_error{
		public:
			fileOpenError();
	};

	void wavWrite(std::vector<float>sound,std::string filename);
	std::vector<float> wavRead(std::string path);
	void append(std::vector<float>sound, std::string filename);

	void write(std::ostream& outFile, int);
	void write(std::ostream& outFile, float);
	void write(std::ostream& outFile, std::vector<int>);
	void write(std::ostream& outFile, std::vector<float>);
	void write(std::ostream& outFile, std::vector<std::vector<float>>);

	
	int                              read(std::istream& inFile, int);
	float                           read(std::istream& inFile, float);
	std::vector<int>              read(std::istream& inFile, std::vector<int>);
	std::vector<float>              read(std::istream& inFile, std::vector<float>);
	std::vector<std::vector<float>> read(std::istream& inFile, std::vector<std::vector<float>>);
}
