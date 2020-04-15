/*
 * Copyright (C) 2020 CESNET
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

#include <atomic>
#include <csignal>
#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <getopt.h>
#include <sys/time.h>

#include <libtrap/trap.h>
#include <unirec/unirec.h>

#include "fields.h"
#include "ZWaveStatistics.h"

UR_FIELDS(
	time TIMESTAMP,
	uint8 CHANNEL,
	bytes FRAME,

	uint64 DEV_ADDR,

	double CORRUPTED_C,
	double CORRUPTED_CH1_C,
	double CORRUPTED_CH2_C,
	double CORRUPTED_CH3_C,
	double TOTAL_OK_C,
	double ROUTED_C,
	double ROUTED_ACK_C,
	double ROUTED_NACK_C,
	double ROUTED_APP_C,
	double SINGLECAST_C,
	double ACK_C,
	double MULTICAST_C,
	double BROADCAST_C,
	double NET_MANAG_C,

	double SRC_NACK_C,
	double DST_NACK_C,
	double FAILED_HOP_NACK_C,
	double SRC_TOTAL_C,
	double DST_TOTAL_C,
	double SRC_SINGL_C,
	double DST_SINGL_C,
	double SRC_ACK_C,
	double DST_ACK_C,
	double SRC_MULTICAST_C,
	double SRC_BROADCAST_C,
	double SRC_TOTAL_LM_T,
	double DST_TOTAL_LM_T,
	double SRC_SINGL_LM_T,
	double DST_SINGL_LM_T,
)

#define INPUT_TEMPLATE "TIMESTAMP, CHANNEL, FRAME"
#define NETWORK_TEMPLATE "TIMESTAMP, DEV_ADDR, CORRUPTED_C, CORRUPTED_CH1_C, CORRUPTED_CH2_C, CORRUPTED_CH3_C, TOTAL_OK_C, ROUTED_C, ROUTED_ACK_C, ROUTED_NACK_C, ROUTED_APP_C, SINGLECAST_C, ACK_C, MULTICAST_C, BROADCAST_C, NET_MANAG_C"
#define NODE_TEMPLATE "TIMESTAMP, DEV_ADDR, SRC_NACK_C, DST_NACK_C, FAILED_HOP_NACK_C, SRC_TOTAL_C, DST_TOTAL_C, SRC_SINGL_C, DST_SINGL_C, SRC_ACK_C, DST_ACK_C, SRC_MULTICAST_C, SRC_BROADCAST_C, SRC_TOTAL_LM_T, DST_TOTAL_LM_T, SRC_SINGL_LM_T, DST_SINGL_LM_T"

trap_module_info_t *module_info = NULL;

#define MODULE_BASIC_INFO(BASIC) \
	BASIC("zwave-stats-creator", "Create statistics from Z-Wave traffic (network statistics and statistics for every communicating node)", 1, 2)

#define MODULE_PARAMS(PARAM) \
	PARAM('s', "stats-interval", "Interval to generate statistics in seconds", required_argument, "uint8") \
	PARAM('n', "network", "HomeID of the network", required_argument, "string") \
	PARAM('t', "testing", "Testing mode (timestamp 0 in statistics)", no_argument, "none") \
	PARAM('I', "ignore-in-eof", "Do not terminate on incomming termination message.", no_argument, "none")

std::atomic_bool g_stop = { false };
TRAP_DEFAULT_SIGNAL_HANDLER(g_stop = true)

int main(int argc, char *argv[])
{
	INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
	TRAP_DEFAULT_INITIALIZATION(argc, argv, *module_info);

	TRAP_REGISTER_DEFAULT_SIGNAL_HANDLER();

	ur_template_t *in_frames_template = NULL;
	ur_template_t *out_network_template = NULL;
	ur_template_t *out_node_template = NULL;
	void *network_record = NULL;
	void *node_record = NULL;
	int opt;
	int stats_interval = 10; // in seconds
	uint32_t home_id = 0;
	bool testing = false;
	std::atomic_bool forward_eof = { false };
	int ignore_eof = 0; // Ignore EOF input parameter flag

	while ((opt = getopt_long(argc, argv, module_getopt_string, long_options, NULL)) != -1) {
		switch (opt) {
		case 's':
			stats_interval = atoi(optarg);
			break;
		case 'n': {
			std::string param(optarg);
			if (param.size() != 8) {
				std::cerr << "Error: HomeID must have 8 characters" << std::endl;
				TRAP_DEFAULT_FINALIZATION();
				FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
				return 1;
			}

			std::stringstream ss;
			ss << std::hex << param;
			ss >> home_id;

			break;
		}
		case 't':
			testing = true;
			break;
		case 'I':
			ignore_eof = 1;
			break;
		default:
			std::cerr << "Invalid argument." << std::endl;
			TRAP_DEFAULT_FINALIZATION();
			FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
			return 1;
		}
	}

	auto cleanup = [&](){
		TRAP_DEFAULT_FINALIZATION();

		if (in_frames_template != NULL) { ur_free_template(in_frames_template); }
		if (out_network_template != NULL) { ur_free_template(out_network_template); }
		if (out_node_template != NULL) { ur_free_template(out_node_template); }

		if (network_record != NULL) { ur_free_record(network_record); }
		if (node_record != NULL) { ur_free_record(node_record); }

		ur_finalize();

		FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
	};

	if (trap_ifcctl(TRAPIFC_INPUT, 0, TRAPCTL_SETTIMEOUT, TRAP_WAIT) != TRAP_E_OK) {
		std::cerr << "Error: could not set input interface timeout." << std::endl;
		cleanup();
		return 1;
	}

	in_frames_template = ur_create_input_template(0, INPUT_TEMPLATE, NULL);
	if (in_frames_template == NULL) {
		std::cerr << "Error: input template could not be created." << std::endl;
		cleanup();
		return 1;
	}

	if (trap_ifcctl(TRAPIFC_OUTPUT, 0, TRAPCTL_SETTIMEOUT, TRAP_WAIT) != TRAP_E_OK) {
		std::cerr << "Error: could not set output network interface timeout." << std::endl;
		cleanup();
		return 1;
	}

	out_network_template = ur_create_output_template(0, NETWORK_TEMPLATE, NULL);
	if (out_network_template == NULL) {
		std::cerr << "Error: output network template could not be created." << std::endl;
		cleanup();
		return 1;
	}

	network_record = ur_create_record(out_network_template, UR_MAX_SIZE);
	if (network_record == NULL) {
		std::cerr << "Error: Memory allocation problem (output network record)." << std::endl;
		cleanup();
		return 1;
	}

	if (trap_ifcctl(TRAPIFC_OUTPUT, 1, TRAPCTL_SETTIMEOUT, TRAP_WAIT) != TRAP_E_OK) {
		std::cerr << "Error: could not set output node interface timeout." << std::endl;
		cleanup();
		return 1;
	}

	out_node_template = ur_create_output_template(1, NODE_TEMPLATE, NULL);
	if (out_node_template == NULL) {
		std::cerr << "Error: output node template could not be created." << std::endl;
		cleanup();
		return 1;
	}

	node_record = ur_create_record(out_node_template, UR_MAX_SIZE);
	if (node_record == NULL) {
		std::cerr << "Error: Memory allocation problem (output node record)." << std::endl;
		cleanup();
		return 1;
	}

	ZWaveStatistics stats(home_id);

	std::mutex mx;
	std::thread t([&]() {
		while (!g_stop)
		{
			std::this_thread::sleep_for(std::chrono::seconds(stats_interval));

			std::lock_guard<std::mutex> lock(mx);

			timeval tv = {};
			::gettimeofday(&tv, nullptr);
			ur_time_t timestamp = 0;
			if (!testing) {
				timestamp = ur_time_from_sec_usec(tv.tv_sec, tv.tv_usec);
			}

			ur_set(out_network_template, network_record, F_TIMESTAMP, timestamp);
			ur_set(out_network_template, network_record, F_DEV_ADDR, home_id);
			ur_set(out_network_template, network_record, F_CORRUPTED_C, stats.network_stats_.corrupted_c);
			ur_set(out_network_template, network_record, F_CORRUPTED_CH1_C, stats.network_stats_.corrupted_ch1_c);
			ur_set(out_network_template, network_record, F_CORRUPTED_CH2_C, stats.network_stats_.corrupted_ch2_c);
			ur_set(out_network_template, network_record, F_CORRUPTED_CH3_C, stats.network_stats_.corrupted_ch3_c);
			ur_set(out_network_template, network_record, F_TOTAL_OK_C, stats.network_stats_.total_ok_c);
			ur_set(out_network_template, network_record, F_ROUTED_C, stats.network_stats_.routed_c);
			ur_set(out_network_template, network_record, F_ROUTED_ACK_C, stats.network_stats_.routed_ack_c);
			ur_set(out_network_template, network_record, F_ROUTED_NACK_C, stats.network_stats_.routed_nack_c);
			ur_set(out_network_template, network_record, F_ROUTED_APP_C, stats.network_stats_.routed_app_c);
			ur_set(out_network_template, network_record, F_ACK_C, stats.network_stats_.ack_c);
			ur_set(out_network_template, network_record, F_SINGLECAST_C, stats.network_stats_.singlecast_c);
			ur_set(out_network_template, network_record, F_MULTICAST_C, stats.network_stats_.multicast_c);
			ur_set(out_network_template, network_record, F_BROADCAST_C, stats.network_stats_.broadcast_c);
			ur_set(out_network_template, network_record, F_NET_MANAG_C, stats.network_stats_.net_manag_c);
			trap_send(0, network_record, ur_rec_size(out_network_template, network_record));

			for (int i = 1; i < MAX_NODES; ++i) {
				const NodeStats &node_stats = stats.nodes_[i];

				if (!node_stats.should_be_reported) { continue; }

				ur_set(out_node_template, node_record, F_TIMESTAMP, timestamp);
				ur_set(out_node_template, node_record, F_DEV_ADDR, i);
				ur_set(out_node_template, node_record, F_SRC_NACK_C, node_stats.src_nack_c);
				ur_set(out_node_template, node_record, F_DST_NACK_C, node_stats.dst_nack_c);
				ur_set(out_node_template, node_record, F_FAILED_HOP_NACK_C, node_stats.failed_hop_nack_c);
				ur_set(out_node_template, node_record, F_SRC_TOTAL_C, node_stats.src_total_c);
				ur_set(out_node_template, node_record, F_DST_TOTAL_C, node_stats.dst_total_c);
				ur_set(out_node_template, node_record, F_SRC_SINGL_C, node_stats.src_singl_c);
				ur_set(out_node_template, node_record, F_DST_SINGL_C, node_stats.dst_singl_c);
				ur_set(out_node_template, node_record, F_SRC_ACK_C, node_stats.src_ack_c);
				ur_set(out_node_template, node_record, F_DST_ACK_C, node_stats.dst_ack_c);
				ur_set(out_node_template, node_record, F_SRC_MULTICAST_C, node_stats.src_multicast_c);
				ur_set(out_node_template, node_record, F_SRC_BROADCAST_C, node_stats.src_broadcast_c);
				ur_set(out_node_template, node_record, F_SRC_TOTAL_LM_T, node_stats.src_total_lm_t);
				ur_set(out_node_template, node_record, F_DST_TOTAL_LM_T, node_stats.dst_total_lm_t);
				ur_set(out_node_template, node_record, F_SRC_SINGL_LM_T, node_stats.src_singl_lm_t);
				ur_set(out_node_template, node_record, F_DST_SINGL_LM_T, node_stats.dst_singl_lm_t);
				trap_send(1, node_record, ur_rec_size(out_node_template, node_record));
			}
		}

		if (forward_eof) {
			char dummy[1] = {0};
			trap_send(0, dummy, 1);
			trap_send_flush(0);
			trap_send(1, dummy, 1);
			trap_send_flush(1);
		}
	});

	while (!g_stop) {
		const void *in_record;
		uint16_t in_record_size;

		int ret = TRAP_RECEIVE(0, in_record, in_record_size, in_frames_template);
		TRAP_DEFAULT_RECV_ERROR_HANDLING(ret, continue, break);

		// EOF close this module
		if (in_record_size <= 1){
			forward_eof = true;
			// if ignore_eof option is used keep this module running
			if (ignore_eof) { continue; }

			break;
		}

		ur_time_t timestamp = ur_get(in_frames_template, in_record, F_TIMESTAMP);
		ZWave::Channel channel = (ZWave::Channel) ur_get(in_frames_template, in_record, F_CHANNEL);
		uint8_t *bytes = (uint8_t *) ur_get_ptr(in_frames_template, in_record, F_FRAME);
		uint8_t size = ur_get_var_len(in_frames_template, in_record, F_FRAME);

		ZWave::FrameWrapper frame(bytes, size, channel);
		frame.setTimestamp(timestamp);

		{
			std::lock_guard<std::mutex> lock(mx);
			stats.processFrame(frame);
		}
	}

	g_stop = true;

	t.join();

	cleanup();
	return 0;
}
