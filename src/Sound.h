#include <vector>
#include <complex>
namespace sound{
	class Sound{
		public:
			int windowLength;
			int hop;
			int hops;
			int overlap;
			std::vector< std::vector<std::complex<double> > > soundData;

			Sound(std::vector<double> pcm, int, int);
			std::vector<double> synthesize();
			int length();
	};
}
