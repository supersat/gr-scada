
#include <scada/filters.h>

#include <math.h>
#include <complex>

using std::complex;
using std::vector;

static complex<double> j(0, 1);

namespace gr {
    namespace scada {
        vector<float>
        filters::cosine_filter(double gain, double sps, int ntaps, bool root)
        {
            // Ensure ntaps is odd
            ntaps |= 1;
            int half_ntaps = ntaps / 2;
            vector<float> taps(ntaps);
            for (int i = 0; i < ntaps; i++) {
                complex<double> sum = 0;
                for (int m = -half_ntaps; m < half_ntaps; m++) {
                    double f = sps * m / ntaps;
                    if (fabs(f) <= .5) {
                        double coeff = cos(M_PI * f); // * .5?
                        if (root) {
                            coeff = sqrt(coeff);
                        }
                        sum += coeff * std::exp(M_PI * 2 * m * j * ((double)(i - half_ntaps) / ntaps));
                    }
                }
                taps[i] = gain * sum.real() / ntaps;
            }

            return taps;
        }
    }
}


