#include <vector>
namespace sound{
	class Sound{
		public:
			int windowLength;
			int hop;
			int hops;
			int overlap;
			std::vector< std::vector<double> > frequencies;
			std::vector< std::vector<double> > phases;

			Sound(std::vector<double> pcm, int, int);
			std::vector<double> synthesize();
			int length();
	};
}
