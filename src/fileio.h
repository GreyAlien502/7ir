#include <vector>
#include <string>
#include <iostream>

namespace fileio{
	class fileOpenError: public std::runtime_error{
		public:
			fileOpenError();
	};

	void wavWrite(std::vector<double>sound,std::string filename);
	std::vector<double> wavRead(std::string path);
	void append(std::vector<double>sound, std::string filename);

	void write(std::ostream& outFile, int);
	void write(std::ostream& outFile, double);
	void write(std::ostream& outFile, std::vector<int>);
	void write(std::ostream& outFile, std::vector<double>);
	void write(std::ostream& outFile, std::vector<std::vector<double>>);

	
	int                              read(std::istream& inFile, int);
	double                           read(std::istream& inFile, double);
	std::vector<int>              read(std::istream& inFile, std::vector<int>);
	std::vector<double>              read(std::istream& inFile, std::vector<double>);
	std::vector<std::vector<double>> read(std::istream& inFile, std::vector<std::vector<double>>);
}
