/*
 * Copyright (C) 2019 CESNET
 *
 * LICENSE TERMS
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * This software is provided ``as is'', and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
*/

#include <complex>
#include <functional>
#include <memory>

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
	std::complex<double> atan_fm_demodulator_s1 = 0;
	float lp_filter1_xv[NZEROS6 + 1] = {};
	float lp_filter1_yv[NPOLES6 + 1] = {};
	float lp_filter2_xv[NZEROS6 + 1] = {};
	float lp_filter2_yv[NPOLES6 + 1] = {};
	float freq_filter_xv[NZEROS + 1] = {};
	float freq_filter_yv[NPOLES + 1] = {};
	float lock_filter_xv[NZEROS1 + 1] = {};
	float lock_filter_yv[NPOLES1 + 1] = {};

	std::unique_ptr<FILE, decltype(&pclose)> pipe;
};
