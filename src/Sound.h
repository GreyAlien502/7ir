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

			Sound(std::vector<double>, int, int);
			std::vector<double> synthesize();
			int length();

			void normalize();
			void transpose(int);
			void lowpass(int);
			void highpass(int);
	};
}
