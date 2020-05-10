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

#include <csignal>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <getopt.h>

#include <libtrap/trap.h>
#include <unirec/unirec.h>

#include "fields.h"
#include "ZWaveRtlSdr.h"
#include "../zwave/ZWave.h"

UR_FIELDS (
	time TIMESTAMP
	uint8 CHANNEL
	bytes FRAME
)

#define UNIREC_TEMPLATE "TIMESTAMP, CHANNEL, FRAME"

trap_module_info_t *module_info = NULL;

#define MODULE_BASIC_INFO(BASIC) \
	BASIC("zwave-sdr-sniffer", \
		"Sniff Z-Wave frames using 2x rtl_sdr on frequencies 868.42 MHz and 869.85 MHz.", 0, 1)

#define MODULE_PARAMS(PARAM) \
	PARAM('g', "gain", "Gain of the dongle", required_argument, "uint8")

static int g_stop = 0;
TRAP_DEFAULT_SIGNAL_HANDLER(g_stop = 1)

void exportFrame(const ZWave::FrameWrapper &frame, ur_template_t *out_template, void *out_record)
{
	ur_set(out_template, out_record, F_TIMESTAMP, frame.timestamp());
	ur_set(out_template, out_record, F_CHANNEL, frame.channel_);
	ur_set_var(out_template, out_record, F_FRAME, frame.bytes_, frame.size_);

	trap_send(0, out_record, ur_rec_size(out_template, out_record));
}

int main(int argc, char *argv[])
{
	INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
	TRAP_DEFAULT_INITIALIZATION(argc, argv, *module_info);

	TRAP_REGISTER_DEFAULT_SIGNAL_HANDLER();

	int opt;
	std::string gain = "30";

	while ((opt = getopt_long(argc, argv, module_getopt_string, long_options, NULL)) != -1) {
		switch (opt) {
		case 'g':
			gain = optarg;
			break;
		default:
			std::cerr << "Invalid argument." << std::endl;
			TRAP_DEFAULT_FINALIZATION();
			FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
			return -1;
		}
	}

	ur_template_t *out_template;
	void *out_record;

	auto cleanup = [&](){
		TRAP_DEFAULT_FINALIZATION();

		if (out_template != NULL) { ur_free_template(out_template); }
		if (out_record != NULL) { ur_free_record(out_record); }

		ur_finalize();

		FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
	};

	out_template = ur_create_output_template(0, UNIREC_TEMPLATE, NULL);
	if (out_template == NULL) {
		std::cerr << "Error: output template could not be created." << std::endl;
		cleanup();
		return 1;
	}

	out_record = ur_create_record(out_template, UR_MAX_SIZE);
	if (out_record == NULL) {
		std::cerr << "Error: Memory allocation problem (output record)." << std::endl;
		cleanup();
		return 1;
	}

	int verbose = trap_get_verbose_level();

	ZWaveRtlSdr rtlSdr868_40("868.40e6", "0", gain);
	ZWaveRtlSdr rtlSdr869_85("869.85e6", "1", gain);

	std::mutex mx;

	auto callback868_40 = [&](uint8_t *data, uint8_t len, bool isManchesterEnc) {
		std::lock_guard<std::mutex> lock(mx);

		ZWave::FrameWrapper frame(data, len, isManchesterEnc ? ZWave::Channel::C1 : ZWave::Channel::C2);
		frame.setTimestampNow();

		if (verbose >= 0) { frame.print(true); }

		exportFrame(frame, out_template, out_record);
	};

	auto callback869_85 = [&](uint8_t *data, uint8_t len, bool isManchesterEnc) {
		std::lock_guard<std::mutex> lock(mx);

		ZWave::FrameWrapper frame(data, len, ZWave::Channel::C3);
		frame.setTimestampNow();

		if (verbose >= 0) { frame.print(true); }

		exportFrame(frame, out_template, out_record);
	};

	std::thread t([&](){
		rtlSdr868_40.run(callback868_40);
	});

	rtlSdr869_85.run(callback869_85);

	t.join();

	cleanup();
	return 0;
}
