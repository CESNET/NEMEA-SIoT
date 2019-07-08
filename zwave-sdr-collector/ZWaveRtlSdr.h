#include <complex>
#include <functional>
#include <memory>

class ZWaveRtlSdr {
public:
	ZWaveRtlSdr(const std::string &frequencyHz, const std::string &device, const std::string &gain);

	void run(std::function<void (uint8_t *data, uint8_t size, bool isManchesterEncoding)> frameCallback);

private:
	inline double atan_fm_demodulator(int re, int im);
	inline double lp_filter1(double in);
	inline double lp_filter2(double in);
	inline double freq_filter(double in);
	inline double lock_filter(double in);

private:
	#define NZEROS6 6
	#define NPOLES6 6
	#define GAIN6   4.570794845e+05

	#define NZEROS 3
	#define NPOLES 3
	#define GAIN   3.681602264e+02

	#define NZEROS1 3
	#define NPOLES1 3
	#define GAIN1   2.856028586e+05

	#define SAMPLERATE 2048000

	std::complex<double> atan_fm_demodulator_s1 = 0;
	float lp_filter1_xv[NZEROS6 + 1];
	float lp_filter1_yv[NPOLES6 + 1];
	float lp_filter2_xv[NZEROS6 + 1];
	float lp_filter2_yv[NPOLES6 + 1];
	float freq_filter_xv[NZEROS + 1];
	float freq_filter_yv[NPOLES + 1];
	float lock_filter_xv[NZEROS1 + 1];
	float lock_filter_yv[NPOLES1 + 1];

	std::unique_ptr<FILE, decltype(&pclose)> pipe;
};
