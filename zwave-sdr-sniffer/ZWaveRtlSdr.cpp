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

#include <string>

#include "ZWaveRtlSdr.h"

using namespace std;

ZWaveRtlSdr::ZWaveRtlSdr(const string &frequencyHz, const string &device, const string &gain):
	pipe(popen(string("rtl_sdr -f " + frequencyHz + " -d " + device + " -g " + gain + " - ").c_str(), "r"), pclose)
{
	if (!pipe)
		throw runtime_error("rtl_sdr is not working");
}

void ZWaveRtlSdr::run(std::function<void (uint8_t *data, uint8_t size, bool isManchesterEncoding)> frameCallback)
{
	struct frame_state {
		unsigned int data_len;
		unsigned char data[64];
		bool last_bit;
		int b_cnt;
		enum {
			B_PREAMP, B_SOF0, B_SOF1, B_DATA
		} state_b;
	} fs;

	enum {
		S_IDLE, S_PREAMP, S_BITLOCK
	} state = S_IDLE;

	int pre_len = 0; // # Length of preamble bit
	int pre_cnt = 0;
	double bit_len = 0;
	double bit_cnt = 0.0;
	double wc = 0; // # center frequency
	bool last_logic = false;
	bool hasSignal = false;
	bool msc = false; //Manchester
	const int lead_in = 10;
	double dr; //Datarate

	double f, s, lock;

	while (!feof(pipe.get())) {
		unsigned char g[1024];
		fread(g, 1024, 1, pipe.get());

		for (int i = 0; i < 1024; i += 2) {
			double re = (g[i] - 127);
			double im = (g[i + 1] - 127);

			re = lp_filter1(re);
			im = lp_filter2(im);

			f = atan_fm_demodulator(re, im);

			s = freq_filter(f);

			/*
			* We use a 12khz lowpass filter to lock on to a preable. When this value is "stable",
			* a preamble could be present, further more the value of lock, will correspond to the
			* center frequency of the fsk (wc)
			*/
			lock = lock_filter(f);

			hasSignal = fabs(lock) > 0.01;

			if (hasSignal) {
				bool logic = (s - wc) < 0;

				if (state == S_IDLE) {
					state = S_PREAMP;
					pre_cnt = 0;
					pre_len = 0;
					wc = lock;
				} else if (state == S_PREAMP) {
					wc = 0.99 * wc + lock * 0.01;
					pre_len++;

					if (logic ^ last_logic) //#edge trigger (rising and falling)
					{
						pre_cnt++;

						if (pre_cnt == lead_in) {//# skip the first lead_in
							pre_len = 0;
						} else if (pre_cnt > lead_in + 20) { //Minimum preamble length is 10 bytes i.e 80 bits
							state = S_BITLOCK;
							fs.state_b = fs.B_PREAMP;
							fs.last_bit = not logic;

							bit_len = double(pre_len) / (pre_cnt - lead_in - 1);
							bit_cnt = 3 * bit_len / 4.0;
							dr = SAMPLERATE / bit_len;
							msc = dr < 15e3; //Should we use manchester encoding
						}
					}
				} else if (state == S_BITLOCK) {
					if (logic ^ last_logic) {
						if (msc && (bit_cnt < bit_len / 2.0)) {
							bit_cnt = 1 * bit_len / 4.0; //#Re-sync on edges
						} else {
							bit_cnt = 3 * bit_len / 4.0; //#Re-sync on edges
						}
					} else {
						bit_cnt = bit_cnt + 1.0;
					}

					if (bit_cnt >= bit_len) {// # new bit
						//Sub state machine
						if (fs.state_b == fs.B_PREAMP) {
							if (logic and fs.last_bit) {
								fs.state_b = fs.B_SOF1;
								fs.b_cnt = 1; //This was the first SOF bit
							}
						} else if (fs.state_b == fs.B_SOF0) {
							if (not logic) {
								if (fs.b_cnt == 4) {
									fs.b_cnt = 0;
									fs.data_len = 0;
									fs.state_b = fs.B_DATA;
								}
							} else {
								state = S_IDLE;
							}
						} else if (fs.state_b == fs.B_SOF1) {
							if (logic) {
								if (fs.b_cnt == 4) {
									fs.b_cnt = 0;
									fs.state_b = fs.B_SOF0;
								}
							} else {
								state = S_IDLE;
							}
						} else if (fs.state_b == fs.B_DATA) {//Payload bit
							fs.data[fs.data_len] =
								(fs.data[fs.data_len] << 1) | logic;

							if ((fs.b_cnt & 7) == 0) {
								fs.data[++fs.data_len] = 0;
							}
						}

						fs.last_bit = logic;
						fs.b_cnt++;
						bit_cnt = bit_cnt - bit_len;
					}
				}

				last_logic = logic;
			} else { //# No LOCKs
				if (state == S_BITLOCK && fs.state_b == fs.B_DATA) {
					frameCallback(fs.data, fs.data_len, msc);
				}

				fs.state_b = fs.B_PREAMP;
				state = S_IDLE;
			}
		}
	}
}

double ZWaveRtlSdr::atan_fm_demodulator(int re, int im)
{
	std::complex<double> s((double) re, (double) im);

	double d = std::arg(std::conj(atan_fm_demodulator_s1) * s);
	atan_fm_demodulator_s1 = s;
	return d;
}

double ZWaveRtlSdr::lp_filter1(double in)
{
	lp_filter1_xv[0] = lp_filter1_xv[1];
	lp_filter1_xv[1] = lp_filter1_xv[2];
	lp_filter1_xv[2] = lp_filter1_xv[3];
	lp_filter1_xv[3] = lp_filter1_xv[4];
	lp_filter1_xv[4] = lp_filter1_xv[5];
	lp_filter1_xv[5] = lp_filter1_xv[6];
	lp_filter1_xv[6] = in / GAIN6;
	lp_filter1_yv[0] = lp_filter1_yv[1];
	lp_filter1_yv[1] = lp_filter1_yv[2];
	lp_filter1_yv[2] = lp_filter1_yv[3];
	lp_filter1_yv[3] = lp_filter1_yv[4];
	lp_filter1_yv[4] = lp_filter1_yv[5];
	lp_filter1_yv[5] = lp_filter1_yv[6];
	lp_filter1_yv[6] = (lp_filter1_xv[0] + lp_filter1_xv[6]) + 6 * (lp_filter1_xv[1] + lp_filter1_xv[5]) + 15 * (lp_filter1_xv[2] + lp_filter1_xv[4])
			+ 20 * lp_filter1_xv[3]
			+ (-0.3862279890 * lp_filter1_yv[0]) + (2.6834487459 * lp_filter1_yv[1])
			+ (-7.8013262392 * lp_filter1_yv[2]) + (12.1514352550 * lp_filter1_yv[3])
			+ (-10.6996337410 * lp_filter1_yv[4]) + (5.0521639483 * lp_filter1_yv[5]);
	return lp_filter1_yv[6];
}

double ZWaveRtlSdr::lp_filter2(double in)
{
	lp_filter2_xv[0] = lp_filter2_xv[1];
	lp_filter2_xv[1] = lp_filter2_xv[2];
	lp_filter2_xv[2] = lp_filter2_xv[3];
	lp_filter2_xv[3] = lp_filter2_xv[4];
	lp_filter2_xv[4] = lp_filter2_xv[5];
	lp_filter2_xv[5] = lp_filter2_xv[6];
	lp_filter2_xv[6] = in / GAIN6;
	lp_filter2_yv[0] = lp_filter2_yv[1];
	lp_filter2_yv[1] = lp_filter2_yv[2];
	lp_filter2_yv[2] = lp_filter2_yv[3];
	lp_filter2_yv[3] = lp_filter2_yv[4];
	lp_filter2_yv[4] = lp_filter2_yv[5];
	lp_filter2_yv[5] = lp_filter2_yv[6];
	lp_filter2_yv[6] = (lp_filter2_xv[0] + lp_filter2_xv[6]) + 6 * (lp_filter2_xv[1] + lp_filter2_xv[5]) + 15 * (lp_filter2_xv[2] + lp_filter2_xv[4])
			+ 20 * lp_filter2_xv[3]
			+ (-0.3862279890 * lp_filter2_yv[0]) + (2.6834487459 * lp_filter2_yv[1])
			+ (-7.8013262392 * lp_filter2_yv[2]) + (12.1514352550 * lp_filter2_yv[3])
			+ (-10.6996337410 * lp_filter2_yv[4]) + (5.0521639483 * lp_filter2_yv[5]);
	return lp_filter2_yv[6];
}

double ZWaveRtlSdr::freq_filter(double in)
{
	freq_filter_xv[0] = freq_filter_xv[1];
	freq_filter_xv[1] = freq_filter_xv[2];
	freq_filter_xv[2] = freq_filter_xv[3];
	freq_filter_xv[3] = in / GAIN;
	freq_filter_yv[0] = freq_filter_yv[1];
	freq_filter_yv[1] = freq_filter_yv[2];
	freq_filter_yv[2] = freq_filter_yv[3];
	freq_filter_yv[3] = (freq_filter_xv[0] + freq_filter_xv[3]) + 3 * (freq_filter_xv[1] + freq_filter_xv[2]) + (0.5400688125 * freq_filter_yv[0])
			+ (-1.9504598825 * freq_filter_yv[1]) + (2.3886614006 * freq_filter_yv[2]);
	return freq_filter_yv[3];
}

double ZWaveRtlSdr::lock_filter(double in)
{
	lock_filter_xv[0] = lock_filter_xv[1];
	lock_filter_xv[1] = lock_filter_xv[2];
	lock_filter_xv[2] = lock_filter_xv[3];
	lock_filter_xv[3] = in / GAIN1;
	lock_filter_yv[0] = lock_filter_yv[1];
	lock_filter_yv[1] = lock_filter_yv[2];
	lock_filter_yv[2] = lock_filter_yv[3];
	lock_filter_yv[3] = (lock_filter_xv[0] + lock_filter_xv[3]) + 3 * (lock_filter_xv[1] + lock_filter_xv[2]) + (0.9404830634 * lock_filter_yv[0])
			+ (-2.8791542471 * lock_filter_yv[1]) + (2.9386431728 * lock_filter_yv[2]);
	return lock_filter_yv[3];
}
