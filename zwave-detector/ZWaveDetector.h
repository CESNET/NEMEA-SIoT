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

#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <set>
#include <time.h>

#include <unirec/unirec.h>

#include "../zwave/Conversion.h"
#include "../zwave/ZWave.h"

#define MAX_NODES 256

class ZWaveDetector {
public:
	enum AlertCode : uint32_t {
		NETWORK_SCANNING = 1,
		MODIFICATION_NL = 2,
		MODIFICATION_SR_CACHE = 3,
		MITM_NODE = 4,
	};

	struct ScanAttack {
		void update(const ur_time_t start)
		{
			if (is_present) { return; }

			is_present = true;
			received_time = ::time(nullptr);
			start_time = start;
		}

		std::string caption()
		{
			std::string caption = "Scanned:";
			caption += (dev_manufacturer_and_dev_type ? " Device Manufacturer and Device Type," : "");
			caption += (dev_soft_version ? " Device Software Version," : "");
			caption += (supported_cc ? " Supported Command Classes," : "");
			caption += (basic_operation_status ? " Basic Operational Status," : "");
			caption += (configuration_settings ? " Configuration Settings," : "");

			return caption;
		}

		bool is_present = false;
		time_t received_time = 0;
		ur_time_t start_time = 0;
		bool dev_manufacturer_and_dev_type = false;
		bool dev_soft_version = false;
		bool supported_cc = false;
		bool basic_operation_status = false;
		bool configuration_settings = false;
	};

	struct ModificationNLAttack {
		void update(const ur_time_t start, const std::set<uint8_t> &nodes)
		{
			if (!is_present) {
				is_present = true;
				received_time = ::time(nullptr);
				start_time = start;
			}

			unpaired_nodes.insert(nodes.begin(), nodes.end());
		}

		std::string caption()
		{
			std::string caption = "Unknown nodes in NL:";
			for (const auto &node: unpaired_nodes)
			{
				caption += " " + to_string((int) node) + ",";
			}
			caption += ".";

			return caption;
		}

		bool is_present = false;
		time_t received_time = 0;
		ur_time_t start_time = 0;
		std::set<uint8_t> unpaired_nodes;
	};

	struct ModificationSRCacheAttack {
		void update(const ur_time_t start, const std::vector<uint8_t> &nodes)
		{
			if (!is_present) {
				is_present = true;
				received_time = ::time(nullptr);
				start_time = start;
			}

			unpaired_nodes.insert(nodes.begin(), nodes.end());
		}

		std::string caption()
		{
			std::string caption = "Unknown nodes in SR Cache:";
			for (const auto &node: unpaired_nodes)
			{
				caption += " " + to_string((int) node) + ",";
			}
			caption += ".";

			return caption;
		}

		bool is_present = false;
		time_t received_time = 0;
		ur_time_t start_time = 0;
		std::set<uint8_t> unpaired_nodes;
	};

	struct MITMNode {
		void update(const ur_time_t start, const std::string &route)
		{
			if (!is_present) {
				is_present = true;
				received_time = ::time(nullptr);
				start_time = start;
			}

			attacked_routes.insert(route);
		}

		std::string caption()
		{
			std::string caption = "Attacked routes:";
			for (const auto &route: attacked_routes)
			{
				caption += " " + route + ",";
			}
			caption += ".";

			return caption;
		}

		bool is_present = false;
		time_t received_time = 0;
		ur_time_t start_time = 0;
		std::set<std::string> attacked_routes;
	};

	struct NodeInfo {
		bool paired = false;
		ur_time_t add_time = 0;
		ur_time_t remove_time = 0;

		ScanAttack scan_attack;
		ModificationNLAttack modification_nl;
		ModificationSRCacheAttack modification_sr_cache;
		MITMNode mitm_node;
	};

	ZWaveDetector(
		int alert_interval,
		int sync_time_window,
		int pairing_time_window,
		bool verbose,
		ur_template_t *alert_template,
		void *alert_record)
		: alert_interval_(alert_interval),
		  sync_time_window_(sync_time_window),
		  pairing_time_window_(pairing_time_window),
		  verbose_(verbose ),
		  alert_template_(alert_template),
		  alert_record_(alert_record),
		  nodes_(MAX_NODES)
	{
	}

	void init(uint32_t home_id);
	bool isHomeIdInitialized();

	void nodeAdded(uint8_t node, ur_time_t timestamp);
	void nodeRemoved(uint8_t node, ur_time_t timestamp);

	void processFrame(const ZWave::FrameWrapper &frame);

	void run(const std::atomic_bool &stop, const std::atomic_bool &send_eof);

private:
	void tryDetectNetworkScanning(const ZWave::FrameWrapper &frame);
	void reportNetworkScanning(const time_t &now, uint8_t node_id);
	// attacks on network routing
	void tryDetectModificationNL(const ZWave::FrameWrapper &frame);
	void reportModificationNL(const time_t &now, uint8_t node_id);

	void tryDetectModificationSRCache(const ZWave::FrameWrapper &frame);
	void reportModificationSRCache(const time_t &now, uint8_t node_id);

	void tryDetectMITM(const ZWave::FrameWrapper &frame);
	void reportMITM(const time_t &now, uint8_t node_id);

private:
	std::mutex mutex_;
	int alert_interval_;
	int sync_time_window_;
	int pairing_time_window_;
	bool verbose_;
	ur_template_t *alert_template_;
	void *alert_record_;
	uint32_t home_id_ = 0;
	std::atomic_bool home_id_initialized_ = { false };
	std::atomic_bool first_frame_received_ = { false };
	std::vector<NodeInfo> nodes_;
};
