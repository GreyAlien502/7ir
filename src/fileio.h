#include <vector>
#include <string>

namespace fileio{
	class fileReadError: public std::runtime_error{
		public:
			std::string filename;

			fileReadError(std::string name);
			const char* what() const noexcept;
	};
	bool save(std::vector<double>sound,std::string filename);
	std::vector<double> read(std::string filename);
}
