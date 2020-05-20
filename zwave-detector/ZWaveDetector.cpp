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

trap_module_info_t *module_info = NULL;

#define MODULE_BASIC_INFO(BASIC) \
	BASIC("zwave-detector", "Detect Z-Wave network scanning and attacks on routing", 2, 1)

#define MODULE_PARAMS(PARAM) \
	PARAM('a', "alert-interval", "Interval to report alerts in seconds", required_argument, "uint8") \
	PARAM('s', "sync-time-window", "Time window for synchronization events and frames", required_argument, "uint8") \
	PARAM('p', "pairing-time-window", "Time window to not report scan attack alert during pairing", required_argument, "uint8") \
	PARAM('n', "network", "HomeID of the network", required_argument, "string") \
	PARAM('I', "ignore-in-eof", "Do not terminate on incomming termination message.", no_argument, "none")

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
	int sync_time_window = 3; // in seconds
	int pairing_time_window = 5; // in seconds
	uint32_t home_id = 0;
	std::atomic_bool forward_eof = { false };
	int ignore_eof = 0; // Ignore EOF input parameter flag

	while ((opt = getopt_long(argc, argv, module_getopt_string, long_options, NULL)) != -1) {
		switch (opt) {
		case 'a':
			alert_interval = atoi(optarg);
			break;
		case 's':
			sync_time_window = atoi(optarg);
			break;
		case 'p':
			pairing_time_window = atoi(optarg);
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
		case 'I':
			ignore_eof = 1;
			break;
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

	if (trap_ifcctl(TRAPIFC_OUTPUT, 0, TRAPCTL_SETTIMEOUT, TRAP_NO_WAIT) != TRAP_E_OK) {
		std::cerr << "Error: could not set output interface timeout." << std::endl;
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

	const bool verbose = trap_get_verbose_level() > 0;

	ZWaveDetector detector(
		alert_interval,
		sync_time_window,
		pairing_time_window,
		verbose,
		out_alerts_template,
		alert_record
	);

	if (home_id != 0) {
		detector.init(home_id);
	}

	std::thread events_thread([&](){
		if (verbose) {
			std::cout << "Events processing thread started." << std::endl;
		}

		while (!g_stop) {
			const void *in_record;
			uint16_t in_record_size;

			int ret = TRAP_RECEIVE(1, in_record, in_record_size, in_events_template);
			TRAP_DEFAULT_RECV_ERROR_HANDLING(ret, continue, break);

			// EOF but do not close this module
			// module will be closed through frames interface
			if (in_record_size <= 1) {
				forward_eof = true;

				if (ignore_eof) { continue; }

				break;
			}

			ur_time_t timestamp = ur_get(in_events_template, in_record, F_TIME);
			double event_type = ur_get(in_events_template, in_record, F_EVENT_TYPE);
			double node_id = ur_get(in_events_template, in_record, F_NODE_ID);

			if (event_type == EVENT_DRIVER_READY) {
				double home_id = ur_get(in_events_template, in_record, F_HOME_ID);
				detector.init(home_id);
			}
			else if (event_type == EVENT_NODE_ADDED) {
				detector.nodeAdded(node_id, timestamp);
			}
			else if (event_type == EVENT_NODE_REMOVED) {
				detector.nodeRemoved(node_id, timestamp);
			}
		}

		if (verbose) {
			std::cout << "Events processing thread ended." << std::endl;
		}
	});

	std::thread frames_thread([&]() {
		if (verbose) {
			std::cout << "Frames processing thread started." << std::endl;
		}

		while (!g_stop) {
			const void *in_record;
			uint16_t in_record_size;

			int ret = TRAP_RECEIVE(0, in_record, in_record_size,
				in_frames_template);
			TRAP_DEFAULT_RECV_ERROR_HANDLING(ret, continue, break);

			// EOF close this module
			if (in_record_size <= 1) {
				forward_eof = true;
				// if ignore_eof option is used keep this module running
				if (ignore_eof) { continue; }

				break;
			}

			if (!detector.isHomeIdInitialized()) { continue; }

			ur_time_t timestamp = ur_get(in_frames_template, in_record, F_TIMESTAMP);
			ZWave::Channel channel = (ZWave::Channel) ur_get(in_frames_template, in_record, F_CHANNEL);
			uint8_t size = ur_get_var_len(in_frames_template, in_record, F_FRAME);
			uint8_t *bytes = (uint8_t *) ur_get_ptr(in_frames_template, in_record, F_FRAME);

			ZWave::FrameWrapper frame(bytes, size, channel);
			frame.setTimestamp(timestamp);

			detector.processFrame(frame);
		}

		g_stop = true;

		if (verbose) {
			std::cout << "Frames processing thread ended." << std::endl;
		}
	});

	detector.run(g_stop, forward_eof);

	events_thread.join();
	frames_thread.join();

	cleanup();
	return 0;
}

void ZWaveDetector::init(uint32_t home_id)
{
	{
		std::lock_guard<std::mutex> lock(mutex_);
		home_id_ = home_id;
	}

	home_id_initialized_ = true;

	if (verbose_) {
		std::cout << "Initialized" << std::endl;
	}
}

bool ZWaveDetector::isHomeIdInitialized()
{
	return home_id_initialized_;
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
	if (!first_frame_received_) {
		first_frame_received_ = true;
	}

	if (!frame.isOK()) { return; }

	if (frame.homeId() != home_id_) { return; }

	if (!frame.isSinglecast()) { return; }

	tryDetectNetworkScanning(frame);

	tryDetectModificationNL(frame);
	tryDetectModificationSRCache(frame);
	tryDetectMITM(frame);
}

void ZWaveDetector::tryDetectNetworkScanning(const ZWave::FrameWrapper &frame)
{
	uint8_t cc;
	uint8_t command;

	if (!frame.getCommandClassAndCommand(cc, command)) { return; }

	uint8_t node = 0;

	std::lock_guard<std::mutex> lock(mutex_);

	switch (cc) {
		case ZWave::CC::ManufacturerSpecific: //device manufacturer and device type
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
			if (verbose_) {
				std::cout
					<< "Potential scan attack: dev man&type: node:" << (int) node
					<< ", timestamp: " << frame.timestamp()
					<< std::endl;
			}
			break;

		case ZWave::CC::Version: //device software version
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
			if (verbose_) {
				std::cout
					<< "Potential scan attack: dev soft version: node:" << (int) node
					<< ", timestamp: " << frame.timestamp()
					<< std::endl;
			}
			break;

		case ZWave::CC::System: //list of supported CCs
			if (command == ZWave::CCSystemCommand::GetSupportedCC) {
				node = frame.dst();
			}
//			//this is sent also after user interaction via sensor button
//			//or after start of sensor
//			else if (command == 0x01) { //node info
//				node = frame.src();
//			}
			else {
				return;
			}

			nodes_[node].scan_attack.update(frame.timestamp());
			nodes_[node].scan_attack.supported_cc = true;
			if (verbose_) {
				std::cout
					<< "Potential scan attack: supported cc: node:" << (int) node
					<< ", timestamp: " << frame.timestamp()
					<< std::endl;
			}
			break;

		case ZWave::CC::Basic: //basic operational status
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
			if (verbose_) {
				std::cout
					<< "Potential scan attack: basic op status: node:" << (int) node
					<< ", timestamp: " << frame.timestamp()
					<< std::endl;
			}
			break;

		case ZWave::CC::Configuration: //configuration settings
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
			if (verbose_) {
				std::cout
					<< "Potential scan attack: configuration: node:" << (int) node
					<< ", timestamp: " << frame.timestamp()
					<< std::endl;
			}
			break;

		default: //other CC - do not detect
			return;
	}
}

void ZWaveDetector::tryDetectModificationNL(const ZWave::FrameWrapper &frame)
{
	uint8_t cc;
	uint8_t command;

	if (!frame.getCommandClassAndCommand(cc, command)) { return; }

	if (cc != ZWave::CC::System) { return; }

	uint8_t node;
	std::set<uint8_t> nl_nodes;
	if (command == ZWave::CCSystemCommand::DoNLTest) {
		node = frame.dst();
		if (!frame.getNodeIdsFromNL(nl_nodes)) { return; }
	}
	else if (command == ZWave::CCSystemCommand::NLTest) {
		node = frame.src();
		nl_nodes.insert(frame.dst());
	}
	else if (command == ZWave::CCSystemCommand::ReportNL) {
		node = frame.src();
		if (!frame.getNodeIdsFromNL(nl_nodes)) { return; }
	}
	else {
		return;
	}

	std::lock_guard<std::mutex> lock(mutex_);

	for (auto it = nl_nodes.begin(); it != nl_nodes.end(); )
	{
		if (nodes_[*it].paired) { it = nl_nodes.erase(it); }
		else { ++it; }
	}

	// all nodes from nl are paired, no attack here
	if (nl_nodes.empty()) { return; }

	nodes_[node].modification_nl.update(frame.timestamp(), nl_nodes);

	if (verbose_) {
		std::cout
			<< "Potential modification nl attack: node:" << (int) node
			<< ", timestamp: " << frame.timestamp()
			<< ", unknown nodes:";
		for (const auto &n: nl_nodes)
		{
			std::cout << " " << (int) n << ", " << std::endl;
		}
		std::cout << std::endl;
	}
}

void ZWaveDetector::tryDetectModificationSRCache(const ZWave::FrameWrapper &frame)
{
	uint8_t cc;
	uint8_t command;

	if (!frame.getCommandClassAndCommand(cc, command)) { return; }

	if (cc != ZWave::CC::System) { return; }

	if (command != ZWave::CCSystemCommand::SRCacheAssignment &&
		command != ZWave::CCSystemCommand::BackboneCacheAssignment) { return; }

	std::vector<uint8_t> hops;
	if (!frame.getHopsFromSRCacheEntry(hops)) { return; }

	std::lock_guard<std::mutex> lock(mutex_);

	for (auto it = hops.begin(); it != hops.end(); )
	{
		if (nodes_[*it].paired) { it = hops.erase(it); }
		else { ++it; }
	}

	// all hops from sr are paired, no attack here
	if (hops.empty()) { return; }

	nodes_[frame.dst()].modification_sr_cache.update(frame.timestamp(), hops);

	if (verbose_) {
		std::cout
			<< "Potential modification sr cache attack: node:" << (int) frame.dst()
			<< ", timestamp: " << frame.timestamp()
			<< ", unknown nodes:";
		for (const auto &n: hops)
		{
			std::cout << " " << (int) n << ", " << std::endl;
		}
		std::cout << std::endl;
	}
}

void ZWaveDetector::tryDetectMITM(const ZWave::FrameWrapper &frame)
{
	if (!frame.isRouted() || !frame.isNetworkHeaderOK()) { return; }

	const uint8_t sr_len = frame.networkHeader()->sr_len;

	std::string route;

	std::lock_guard<std::mutex> lock(mutex_);

	for (uint8_t i = 0; i < sr_len; ++i)
	{
		const uint8_t node = *(frame.networkHops() + i);
		if (nodes_[node].paired) { continue; }

		if (route.empty()) {
			route = frame.constructRouteString();
		}

		nodes_[node].mitm_node.update(frame.timestamp(), route);

		if (verbose_) {
			std::cout
				<< "Potential MITM Node:" << (int) node
				<< ", timestamp: " << frame.timestamp()
				<< ", route: " << route
				<< std::endl;
		}
	}
}

void ZWaveDetector::reportNetworkScanning(const time_t &now, uint8_t node_id)
{
	auto &info = nodes_[node_id];

	if (now - info.scan_attack.received_time >= sync_time_window_) {
		//generate alert just if device was not paired in scan interval
		auto diff_in_sec = ur_timediff(info.scan_attack.start_time, info.add_time) / 1000;
		if (diff_in_sec >= pairing_time_window_) {

			std::string caption = info.scan_attack.caption();

			if (verbose_) {
				std::cout
					<< "Scan Attack: node: " << (int) node_id
					<< " start_time: "
					<< ur_time_get_sec(info.scan_attack.start_time)
					<< ", caption: " << caption
					<< std::endl;
			}

			ur_set(alert_template_, alert_record_, F_TIMESTAMP, info.scan_attack.start_time);
			ur_set(alert_template_, alert_record_, F_INCIDENT_DEV_ADDR, node_id);
			ur_set(alert_template_, alert_record_, F_ALERT_CODE, AlertCode::NETWORK_SCANNING);
			ur_set_string(alert_template_, alert_record_, F_CAPTION, caption.c_str());
			trap_send(0, alert_record_, ur_rec_size(alert_template_, alert_record_));
		}

		info.scan_attack = ScanAttack();
	}
}

void ZWaveDetector::reportModificationNL(const time_t &now, uint8_t node_id)
{
	auto &info = nodes_[node_id];

	if (now - info.modification_nl.received_time >= sync_time_window_) {
		auto &unpaired_nodes = info.modification_nl.unpaired_nodes;

		for (auto it = unpaired_nodes.begin(); it != unpaired_nodes.end(); )
		{
			if (nodes_[*it].paired) { it = unpaired_nodes.erase(it); }
			else { ++it; }
		}

		if (!unpaired_nodes.empty()) {
			std::string caption = info.modification_nl.caption();

			if (verbose_) {
				std::cout
					<< "Modification NL Attack: node: " << (int) node_id
					<< " start_time: "
					<< ur_time_get_sec(info.modification_nl.start_time)
					<< ", caption: " << caption
					<< std::endl;
			}

			ur_set(alert_template_, alert_record_, F_TIMESTAMP, info.modification_nl.start_time);
			ur_set(alert_template_, alert_record_, F_INCIDENT_DEV_ADDR, node_id);
			ur_set(alert_template_, alert_record_, F_ALERT_CODE, AlertCode::MODIFICATION_NL);
			ur_set_string(alert_template_, alert_record_, F_CAPTION, caption.c_str());
			trap_send(0, alert_record_, ur_rec_size(alert_template_, alert_record_));
		}

		info.modification_nl = ModificationNLAttack();
	}
}

void ZWaveDetector::reportModificationSRCache(const time_t &now, uint8_t node_id)
{
	auto &info = nodes_[node_id];

	if (now - info.modification_sr_cache.received_time >= sync_time_window_) {
		auto &unpaired_nodes = info.modification_sr_cache.unpaired_nodes;

		for (auto it = unpaired_nodes.begin(); it != unpaired_nodes.end(); )
		{
			if (nodes_[*it].paired) { it = unpaired_nodes.erase(it); }
			else { ++it; }
		}

		if (!unpaired_nodes.empty()) {
			std::string caption = info.modification_sr_cache.caption();

			if (verbose_) {
				std::cout
					<< "Modification SR Cache Attack: node: " << (int) node_id
					<< " start_time: "
					<< ur_time_get_sec(info.modification_sr_cache.start_time)
					<< ", caption: " << caption
					<< std::endl;
			}

			ur_set(alert_template_, alert_record_, F_TIMESTAMP, info.modification_sr_cache.start_time);
			ur_set(alert_template_, alert_record_, F_INCIDENT_DEV_ADDR, node_id);
			ur_set(alert_template_, alert_record_, F_ALERT_CODE, AlertCode::MODIFICATION_SR_CACHE);
			ur_set_string(alert_template_, alert_record_, F_CAPTION, caption.c_str());
			trap_send(0, alert_record_, ur_rec_size(alert_template_, alert_record_));
		}

		info.modification_sr_cache = ModificationSRCacheAttack();
	}
}
void ZWaveDetector::reportMITM(const time_t &now, uint8_t node_id)
{
	auto &info = nodes_[node_id];

	if (now - info.mitm_node.received_time >= sync_time_window_) {
		if (!info.paired) {
			std::string caption = info.mitm_node.caption();

			if (verbose_) {
				std::cout
					<< "MITM Node: " << (int) node_id
					<< " start_time: "
					<< ur_time_get_sec(info.mitm_node.start_time)
					<< ", caption: " << caption
					<< std::endl;
			}

			ur_set(alert_template_, alert_record_, F_TIMESTAMP, info.mitm_node.start_time);
			ur_set(alert_template_, alert_record_, F_INCIDENT_DEV_ADDR, node_id);
			ur_set(alert_template_, alert_record_, F_ALERT_CODE, AlertCode::MITM_NODE);
			ur_set_string(alert_template_, alert_record_, F_CAPTION, caption.c_str());
			trap_send(0, alert_record_, ur_rec_size(alert_template_, alert_record_));
		}

		info.mitm_node = MITMNode();
	}
}

void ZWaveDetector::run(const std::atomic_bool &stop, const std::atomic_bool &send_eof)
{
	while(!stop && !home_id_initialized_ && !first_frame_received_)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	if (verbose_) {
		std::cout << "Alerts reporting started." << std::endl;
	}

	while (!stop)
	{
		std::this_thread::sleep_for(std::chrono::seconds(alert_interval_));

		time_t now = ::time(NULL);

		std::lock_guard<std::mutex> lock(mutex_);

		for (int i = 1; i < MAX_NODES; ++i)
		{
			auto &info = nodes_[i];

			if (info.scan_attack.is_present) {
				reportNetworkScanning(now, i);
			}
			if (info.modification_nl.is_present) {
				reportModificationNL(now, i);
			}
			if (info.modification_sr_cache.is_present) {
				reportModificationSRCache(now, i);
			}
			if (info.mitm_node.is_present) {
				reportMITM(now, i);
			}
		}
	}

	if (verbose_) {
		std::cout << "Alerts reporting ended." << std::endl;
	}

	if (send_eof) {
		char dummy[1] = {0};
		trap_send(0, dummy, 1);
		trap_send_flush(0);
	}
}
