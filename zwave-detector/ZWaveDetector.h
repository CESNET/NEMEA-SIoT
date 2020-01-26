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

#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <time.h>

#include <unirec/unirec.h>

#include "../zwave/ZWave.h"

#define MAX_NODES 256

class ZWaveDetector {
public:
	enum AlertCode : uint32_t {
		NETWORK_SCANNING = 1
	};

	struct ScanAttack {
		void update(ur_time_t start)
		{
			if (is_present) { return; }

			is_present = true;
			received_time = time(NULL);
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

	struct NodeInfo {
		bool paired = false;
		ur_time_t add_time = 0;
		ur_time_t remove_time = 0;

		ScanAttack scan_attack;
	};


	ZWaveDetector(
		int alert_interval,
		bool verbose,
		ur_template_t *alert_template,
		void *alert_record)
		: alert_interval_(alert_interval),
		  verbose_(verbose ),
		  alert_template_(alert_template),
		  alert_record_(alert_record),
		  nodes_(MAX_NODES)
	{
	}

	void init(uint32_t home_id);
	bool isInitialized();

	void nodeAdded(uint8_t node, ur_time_t timestamp);
	void nodeRemoved(uint8_t node, ur_time_t timestamp);

	void processFrame(const ZWave::FrameWrapper &frame);

	void run(std::atomic_bool &stop);

private:
	void tryNetworkScanning(const ZWave::FrameWrapper &frame);

private:
	std::mutex mutex_;
	int alert_interval_;
	bool verbose_;
	ur_template_t *alert_template_;
	void *alert_record_;
	std::atomic_bool is_initialized_ = { false };
	uint32_t home_id_ = 0;
	std::vector<NodeInfo> nodes_;
};
