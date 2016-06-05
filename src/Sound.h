#include <vector>
#include <complex>
namespace sound{
	class Sound{
		public:
			int windowLength;
			int overlap;
			int hop;
			int hops;
			std::vector< std::vector<double> > magnitudes;
			std::vector< std::vector<double> > frequencies;

			Sound(std::vector<double>, int overlap=16, int windowSize=2048);
			std::vector<double> synthesize();

			int length();

			void transpose(double);
			void transpose(std::vector<double>);
			void lowpass(double);
			void highpass(double);
			void append(Sound);
	};
}
