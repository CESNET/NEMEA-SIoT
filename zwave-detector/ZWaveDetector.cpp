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
#include <thread>
#include <getopt.h>

#include <libtrap/trap.h>
#include <unirec/unirec.h>

#include "fields.h"
#include "ZWaveDetector.h"
#include "../zwave/Conversion.h"

UR_FIELDS(
	bytes   FRAME
	time    TIMESTAMP
	uint8   CHANNEL

	time    TIME
	double  EVENT_TYPE
	double  HOME_ID
	double  NODE_ID

	uint64  INCIDENT_DEV_ADDR
	uint32  ALERT_CODE
	string  CAPTION
)

#define IN_FRAMES_TEMPLATE "TIMESTAMP, CHANNEL, FRAME"
#define IN_EVENTS_TEMPLATE "TIME, EVENT_TYPE, HOME_ID, NODE_ID"
#define OUT_ALERTS_TEMPLATE "TIMESTAMP, INCIDENT_DEV_ADDR, ALERT_CODE, CAPTION"

#define EVENT_NODE_ADDED 6
#define EVENT_NODE_REMOVED 7
#define EVENT_DRIVER_READY 18
#define EVENT_DRIVER_RESET 20
#define EVENT_DRIVER_REMOVED 27

trap_module_info_t *module_info = NULL;

#define MODULE_BASIC_INFO(BASIC) \
	BASIC("zwave-detector", "Detect Z-Wave network scanning and attacks on routing", 2, 1)

#define MODULE_PARAMS(PARAM) \
	PARAM('a', "alert-interval", "Interval to generate alerts in seconds", required_argument, "uint8") \
	PARAM('n', "network", "HomeID of the network", required_argument, "string") \

std::atomic_bool g_stop = { false };
TRAP_DEFAULT_SIGNAL_HANDLER(g_stop = true)

int main(int argc, char *argv[])
{
	INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
	TRAP_DEFAULT_INITIALIZATION(argc, argv, *module_info);

	TRAP_REGISTER_DEFAULT_SIGNAL_HANDLER();

	ur_template_t *in_frames_template = NULL;
	ur_template_t *in_events_template = NULL;
	ur_template_t *out_alerts_template = NULL;
	void *alert_record = NULL;
	int opt;
	int alert_interval = 10; // in seconds
	uint32_t home_id = 0;

	while ((opt = getopt_long(argc, argv, module_getopt_string, long_options, NULL)) != -1) {
		switch (opt) {
		case 'a':
			alert_interval = atoi(optarg);
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
			ss << hex << param;
			ss >> home_id;

			break;
		}
		default:
			std::cerr << "Error: Invalid argument." << std::endl;
			TRAP_DEFAULT_FINALIZATION();
			FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
			return 1;
		}
	}

	auto cleanup = [&](){
		TRAP_DEFAULT_FINALIZATION();

		if (in_frames_template != NULL) { ur_free_template(in_frames_template); }
		if (in_events_template != NULL) { ur_free_template(in_events_template); }
		if (out_alerts_template != NULL) { ur_free_template(out_alerts_template); }
		if (alert_record != NULL) { ur_free_record(alert_record); }

		ur_finalize();

		FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
	};

	if (trap_ifcctl(TRAPIFC_INPUT, 0, TRAPCTL_SETTIMEOUT, TRAP_WAIT) != TRAP_E_OK) {
		std::cerr << "Error: could not set input frames interface timeout." << std::endl;
		cleanup();
		return 1;
	}

	in_frames_template = ur_create_input_template(0, IN_FRAMES_TEMPLATE, NULL);
	if (in_frames_template == NULL) {
		std::cerr << "Error: input frames template could not be created." << std::endl;
		cleanup();
		return 1;
	}

	if (trap_ifcctl(TRAPIFC_INPUT, 1, TRAPCTL_SETTIMEOUT, TRAP_WAIT) != TRAP_E_OK) {
		std::cerr << "Error: could not set input events interface timeout." << std::endl;
		cleanup();
		return 1;
	}

	in_events_template = ur_create_input_template(1, IN_EVENTS_TEMPLATE, NULL);
	if (in_events_template == NULL) {
		std::cerr << "Error: input events template could not be created." << std::endl;
		cleanup();
		return 1;
	}

	out_alerts_template = ur_create_output_template(0, OUT_ALERTS_TEMPLATE, NULL);
	if (out_alerts_template == NULL) {
		cerr << "Error: output alerts template could not be created." << endl;
		cleanup();
		return 1;
	}

	alert_record = ur_create_record(out_alerts_template, UR_MAX_SIZE);
	if (alert_record == NULL) {
		cerr << "Error: Memory allocation problem (alert record).";
		cleanup();
		return 1;
	}

	bool verbose = trap_get_verbose_level() > 0;

	ZWaveDetector detector(alert_interval, verbose, out_alerts_template, alert_record);

	if (home_id != 0) {
		detector.init(home_id);
	}

	std::thread events_thread([&](){
		while (!g_stop) {
			const void *in_record;
			u_int16_t in_record_size;

			int ret = TRAP_RECEIVE(1, in_record, in_record_size, in_events_template);
			TRAP_DEFAULT_RECV_ERROR_HANDLING(ret, continue, break);

			//TODO
			if (in_record_size <= 1) { break; }

			auto timestamp = ur_get(in_events_template, in_record, F_TIME);
			auto event_type = ur_get(in_events_template, in_record, F_EVENT_TYPE);
			auto node_id = ur_get(in_events_template, in_record, F_NODE_ID);

			if (event_type == EVENT_DRIVER_READY) {
				auto home_id = ur_get(in_events_template, in_record, F_HOME_ID);
				detector.init(home_id);
			}
			else if (event_type == EVENT_NODE_ADDED) {
				detector.nodeAdded(node_id, timestamp);
			}
			else if (event_type == EVENT_NODE_REMOVED) {
				detector.nodeRemoved(node_id, timestamp);
			}
		}
	});

	std::thread frames_thread([&]() {
		while (!g_stop) {
			const void *in_record;
			u_int16_t in_record_size;

			int ret = TRAP_RECEIVE(0, in_record, in_record_size,
				in_frames_template);
			TRAP_DEFAULT_RECV_ERROR_HANDLING(ret, continue, break);

			//TODO
			if (in_record_size <= 1) { break; }

			if (!detector.isInitialized()) { continue; }

			ur_time_t timestamp = ur_get(in_frames_template, in_record, F_TIMESTAMP);
			ZWave::Channel channel = (ZWave::Channel) ur_get(in_frames_template, in_record, F_CHANNEL);
			uint8_t size = ur_get_var_len(in_frames_template, in_record, F_FRAME);
			uint8_t *bytes = (uint8_t *) ur_get_ptr(in_frames_template, in_record, F_FRAME);

			ZWave::FrameWrapper frame(bytes, size, channel);
			frame.setTimestamp(timestamp);

			detector.processFrame(frame);
		}
	});

	detector.run(g_stop);

	events_thread.join();
	frames_thread.join();

	cleanup();
	return 0;
}

void ZWaveDetector::init(uint32_t home_id)
{
	std::lock_guard<std::mutex> lock(mutex_);

	home_id_ = home_id;
	is_initialized_ = true;

	if (verbose_) {
		std::cout << "Initialized" << std::endl;
	}
}

bool ZWaveDetector::isInitialized()
{
	return is_initialized_;
}

void ZWaveDetector::nodeAdded(uint8_t node, ur_time_t timestamp)
{
	std::lock_guard<std::mutex> lock(mutex_);

	auto &info = nodes_[node];
	info.paired = true;
	info.add_time = timestamp;

	if (verbose_) {
		std::cout << "Node " << (int) node << " added " << ur_time_get_sec(info.add_time) << std::endl;
	}
}

void ZWaveDetector::nodeRemoved(uint8_t node, ur_time_t timestamp)
{
	std::lock_guard<std::mutex> lock(mutex_);

	auto &info = nodes_[node];
	info.paired = false;
	info.remove_time = timestamp;

	if (verbose_) {
		std::cout << "Node " << (int) node << " removed " << ur_time_get_sec(info.add_time) << std::endl;
	}
}

void ZWaveDetector::processFrame(const ZWave::FrameWrapper &frame)
{
	if (!frame.isOK()) { return; }

	if (frame.homeId() != home_id_) { return; }

	if (!frame.isSinglecast()) { return; }

	tryNetworkScanning(frame);
}

void ZWaveDetector::tryNetworkScanning(const ZWave::FrameWrapper &frame)
{
	//Payload
	uint8_t begin = frame.payloadPos();
	uint8_t end = frame.checksumPos();
	uint8_t payloadLength = end - begin;

	//we need command class and command for our detections
	if (payloadLength < 2) { return; }

	uint8_t command_class = frame.bytes_[begin];
	uint8_t command = frame.bytes_[begin + 1];

	uint8_t node = 0;
	//TODO maybe check also that device responded

	std::lock_guard<std::mutex> lock(mutex_);

	switch (command_class) {
		case 0x72: //device manufacturer and device type
			if (command == 0x04) { //get
				node = frame.dst();
			}
			else if (command == 0x05) { //report
				node = frame.src();
			}
			else {
				return;
			}

			nodes_[node].scan_attack.update(frame.timestamp());
			nodes_[node].scan_attack.dev_manufacturer_and_dev_type = true;
			break;

		case 0x86: //device software version
			if (command == 0x11) { //get
				node = frame.dst();
			}
			else if (command == 0x12) { //report
				node = frame.src();
			}
			else {
				return;
			}

			nodes_[node].scan_attack.update(frame.timestamp());
			nodes_[node].scan_attack.dev_soft_version = true;
			break;

		case 0x01: //system cc - list of supported CC
			if (command == 0x02) { //get supported CC
				node = frame.dst();
			}
				//TODO detect this?
				//this is sent also after user interaction via sensor button
				//or after start of sensor
//			else if (command == 0x01) { //node info
//				node = frame.src();
//			}
			else {
				return;
			}

			nodes_[node].scan_attack.update(frame.timestamp());
			nodes_[node].scan_attack.supported_cc = true;
			break;

		case 0x20: //basic operational status
			if (command == 0x02) { //get
				node = frame.dst();
			}
			else if (command == 0x03) { //report
				node = frame.src();
			}
			else {
				return;
			}

			nodes_[node].scan_attack.update(frame.timestamp());
			nodes_[node].scan_attack.basic_operation_status = true;
			break;

		case 0x70: //configuration settings
			if (command == 0x05) { //get
				node = frame.dst();
			}
			else if (command == 0x06) { //report
				node = frame.src();
			}
			else {
				return;
			}

			nodes_[node].scan_attack.update(frame.timestamp());
			nodes_[node].scan_attack.configuration_settings = true;
			break;

		default: //other CC - do not detect
			return;
	}
}

void ZWaveDetector::run(std::atomic_bool &stop)
{
	while (!stop) {
		if (!isInitialized()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}

		std::this_thread::sleep_for(std::chrono::seconds(alert_interval_));

		time_t now = time(NULL);

		std::lock_guard<std::mutex> lock(mutex_);

		for (int i = 1; i < MAX_NODES; ++i) {
			auto &info = nodes_[i];

			if (!info.scan_attack.is_present) { continue; }
			//reserve in seconds for sync with pairing
			if (now - info.scan_attack.received_time < 5) { continue; }

			//generate alert just if device was not paired in scan interval
			auto diff_in_sec = ur_timediff(info.scan_attack.start_time, info.add_time) / 1000;
			if (diff_in_sec >= 5) {

				std::string caption = info.scan_attack.caption();

				if (verbose_) {
					std::cout
						<< "Scan Attack: Node: " << i
						<< " start_time: "
						<< ur_time_get_sec(info.scan_attack.start_time)
						<< " " << caption
						<< std::endl;
				}

				ur_set(alert_template_, alert_record_, F_TIMESTAMP, info.scan_attack.start_time);
				ur_set(alert_template_, alert_record_, F_INCIDENT_DEV_ADDR, i);
				ur_set(alert_template_, alert_record_, F_ALERT_CODE, AlertCode::NETWORK_SCANNING);
				ur_set_string(alert_template_, alert_record_, F_CAPTION, caption.c_str());
				trap_send(0, alert_record_, ur_rec_size(alert_template_, alert_record_));
			}

			info.scan_attack = ScanAttack();

		}
	}
}
