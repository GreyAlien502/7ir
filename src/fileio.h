#include <vector>

namespace fileio{
	bool save(std::vector<double>sound,char* filename);
	std::vector<double> read(const char* filename);
}
