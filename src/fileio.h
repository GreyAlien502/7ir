#include <vector>
#include <string>
#include <iostream>

namespace fileio{
	class fileOpenError: public std::runtime_error{
		public:
			fileOpenError();
	};

	bool wavWrite(std::vector<double>sound,std::string filename);
	std::vector<double> wavRead(std::string path);
	void append(std::vector<double>sound, std::string filename);
}
