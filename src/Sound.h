#include <vector>
namespace sound{
	class Sound{
		public:
			int hop;
			int hops;
			std::vector< std::vector<double> > frequencies;
			std::vector< std::vector<double> > phases;

			Sound(std::vector<double> pcm);
			std::vector<double> synthesize();
	};
}
