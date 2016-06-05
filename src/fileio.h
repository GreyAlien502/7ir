#include <vector>

namespace fileio{
	bool save(std::vector<double>sound,std::string filename);
	std::vector<double> read(std::string filename);
}
