/**
 * @file ZWaveCollector.cpp
 * @brief
 * @author Jozef Halaj <xhalaj03@stud.fit.vutbr.cz>
 * @date 2019
 */

/*
 * Copyright (C) 2018 CESNET
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
#include <getopt.h>

#include <libtrap/trap.h>
#include <unirec/unirec.h>

#include "fields.h"
#include "../zwave/ZWave.h"

UR_FIELDS(
	bytes   FRAME
	time    TIMESTAMP
	uint8   CHANNEL
	uint64  DEV_ADDR
	uint32  HOME_ID
	uint8   DST_ID
	uint8   SIZE
	string  TYPE
	uint8   ACK_REQ
	uint8   SEQ_NUM
	uint8   ROUTED
	string  ROUTED_TYPE
	uint8   SRC_HOP
	uint8   DST_HOP
	uint8   FAILED_HOP
	bytes   HOPS
	bytes   PAYLOAD
	string  CMD_CLASS_STR
	string  CMD_STR
)

#define INPUT_TEMPLATE "TIMESTAMP, CHANNEL, FRAME"
#define OUTPUT_TEMPLATE "TIMESTAMP, CHANNEL, DEV_ADDR, HOME_ID, DST_ID, SIZE, TYPE, ACK_REQ, ROUTED, ROUTED_TYPE, SRC_HOP, DST_HOP, FAILED_HOP, HOPS, CMD_CLASS_STR, CMD_STR, PAYLOAD"

trap_module_info_t *module_info = NULL;

//TODO help
#define MODULE_BASIC_INFO(BASIC) \
	BASIC("zwave-collector", \
		"TODO HELP", 1, 1)

#define MODULE_PARAMS(PARAM)

static int g_stop = 0;
TRAP_DEFAULT_SIGNAL_HANDLER(g_stop = 1)

using namespace std;

void processFrame(const ZWave::FrameWrapper &frame, ur_template_t *out_template, void *out_record)
{
	if (!frame.isOK())
		return;

	//drop multicast and unknown frames
	if (!frame.isSinglecast() && !frame.isAck())
		return;

	ur_set(out_template, out_record, F_TIMESTAMP, frame.timestamp());
	ur_set(out_template, out_record, F_CHANNEL, frame.channel_);
	ur_set(out_template, out_record, F_DEV_ADDR, frame.src());
	ur_set(out_template, out_record, F_DST_ID, frame.dst());
	ur_set(out_template, out_record, F_HOME_ID, frame.homeId());
	ur_set(out_template, out_record, F_SIZE, frame.length());
	ur_set(out_template, out_record, F_ACK_REQ, frame.isRequest());
	ur_set(out_template, out_record, F_SEQ_NUM, frame.seqNumber());
	ur_set_string(out_template, out_record, F_TYPE, frame.headerTypeStr().c_str());

	ur_set(out_template, out_record, F_ROUTED, frame.isRouted());
	if (frame.isRouted() && frame.isNetworkHeaderOK())
	{
		ur_set_string(out_template, out_record, F_ROUTED_TYPE, frame.routedTypeStr().c_str());
		ur_set(out_template, out_record, F_SRC_HOP, frame.srcHopId());
		ur_set(out_template, out_record, F_DST_HOP, frame.dstHopId());
		ur_set(out_template, out_record, F_FAILED_HOP, frame.failedHopId());
		ur_set_var(out_template, out_record, F_HOPS, frame.networkHops(), frame.networkHeader()->sr_len);
	}
	else
	{
		ur_set_string(out_template, out_record, F_ROUTED_TYPE, "");
		ur_set(out_template, out_record, F_SRC_HOP, 0);
		ur_set(out_template, out_record, F_DST_HOP, 0);
		ur_set(out_template, out_record, F_FAILED_HOP, 0);
		ur_set_var(out_template, out_record, F_HOPS, NULL, 0);
	}

	if (frame.isAck())
	{
		//empty payload
		ur_set_var(out_template, out_record, F_PAYLOAD, NULL, 0);
		ur_set_string(out_template, out_record, F_CMD_CLASS_STR, "");
		ur_set_string(out_template, out_record, F_CMD_STR, "");

		trap_send(0, out_record, ur_rec_size(out_template, out_record));
		return;
	}

	//PAYLOAD
	uint8_t begin = frame.payloadPos();
	uint8_t end = frame.checksumPos();
	uint8_t payloadLength = end - begin;

	ur_set_var(out_template, out_record, F_PAYLOAD, frame.bytes_ + begin, payloadLength);

	ur_set_string(out_template, out_record, F_CMD_CLASS_STR,
		payloadLength >= 1
		? ZWave::commandClassValToStr(frame.bytes_[begin]).c_str()
		: ""
	);
	ur_set_string(out_template, out_record, F_CMD_STR,
		payloadLength >= 2 && frame.bytes_[begin] == 0x01 //CC System
		? ZWave::systemCommandValToStr(frame.bytes_[begin + 1]).c_str()
		: ""
	);

	trap_send(0, out_record, ur_rec_size(out_template, out_record));
}

int main(int argc, char *argv[])
{
	INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
	TRAP_DEFAULT_INITIALIZATION(argc, argv, *module_info);

	TRAP_REGISTER_DEFAULT_SIGNAL_HANDLER();

	ur_template_t *input_template = NULL;
	ur_template_t *output_template = NULL;
	void *output_record = NULL;

	auto cleanup = [&](){
		TRAP_DEFAULT_FINALIZATION();

		if (input_template != NULL) { ur_free_template(input_template); }
		if (output_template != NULL) { ur_free_template(output_template); }

		if (output_record != NULL) { ur_free_record(output_record); }

		ur_finalize();

		FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
	};

	if (trap_ifcctl(TRAPIFC_INPUT, 0, TRAPCTL_SETTIMEOUT, TRAP_WAIT) != TRAP_E_OK) {
		cerr << "Error: could not set input interface timeout." << endl;
		cleanup();
		return 1;
	}

	if (trap_ifcctl(TRAPIFC_OUTPUT, 0, TRAPCTL_SETTIMEOUT, TRAP_NO_WAIT) != TRAP_E_OK) {
		cerr << "Error: could not set output interface timeout." << endl;
		cleanup();
		return 1;
	}

	ur_template_t *in_template = ur_create_input_template(0, INPUT_TEMPLATE, NULL);
	if (in_template == NULL) {
		cerr << "Error: input template could not be created." << endl;
		cleanup();
		return 1;
	}

	output_template = ur_create_output_template(0, OUTPUT_TEMPLATE, NULL);
	if (output_template == NULL) {
		std::cerr << "Error: output network template could not be created." << std::endl;
		cleanup();
		return 1;
	}

	output_record = ur_create_record(output_template, UR_MAX_SIZE);
	if (output_record == NULL) {
		std::cerr << "Error: Memory allocation problem (output network record)." << std::endl;
		cleanup();
		return 1;
	}

	while (!g_stop) {
		const void *in_record;
		u_int16_t in_record_size;

		int ret = TRAP_RECEIVE(0, in_record, in_record_size, in_template);
		TRAP_DEFAULT_RECV_ERROR_HANDLING(ret, continue, break);

		//TODO
		if (in_record_size <= 1) { break; }

		auto timestamp = ur_get(in_template, in_record, F_TIMESTAMP);
		auto channel = (ZWave::Channel) ur_get(in_template, in_record, F_CHANNEL);
		auto *bytes = (uint8_t *) ur_get_ptr(in_template, in_record, F_FRAME);
		auto size = ur_get_var_len(in_template, in_record, F_FRAME);

		ZWave::FrameWrapper frame(bytes, size, channel);
		frame.setTimestamp(timestamp);

		processFrame(frame, output_template, output_record);
	}

	cleanup();
	return 0;
}
