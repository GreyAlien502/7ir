#include <vector>
#include <string>

namespace fileio{
	class fileOpenError: public std::runtime_error{
		public:

			fileOpenError();
			const char* what() const noexcept;
	};

	bool write(std::vector<double>sound,std::string filename);
	void append(std::vector<double>sound, std::string filename);
	std::vector<double> read(std::string filename);
}
