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

#include <vector>

#include "../zwave/ZWave.h"

#define MAX_NODES 256

struct NetworkStats {
	double corrupted_c = 0;
	double corrupted_ch2_c = 0;
	double corrupted_ch1_c = 0;
	double corrupted_ch3_c = 0;
	double total_ok_c = 0;
	double routed_c = 0;
	double routed_ack_c = 0;
	double routed_nack_c = 0;
	double routed_app_c = 0;
	double singlecast_c = 0;
	double ack_c = 0;
	double multicast_c = 0;
	double broadcast_c = 0;
	double net_manag_c = 0;
};

struct NodeStats {
	double src_nack_c = 0;
	double dst_nack_c = 0;
	double failed_hop_nack_c = 0;
	double src_total_c = 0;
	double dst_total_c = 0;
	double src_singl_c = 0;
	double dst_singl_c = 0;
	double src_ack_c = 0;
	double dst_ack_c = 0;
	double src_multicast_c = 0;
	double src_broadcast_c = 0;
	double src_total_lm_t = 0;
	double dst_total_lm_t = 0;
	double src_singl_lm_t = 0;
	double dst_singl_lm_t = 0;

	bool should_be_reported = false;
};

class ZWaveStatistics {
public:
	ZWaveStatistics(uint32_t home_id)
	: home_id_(home_id),
	  nodes_(MAX_NODES)
	{
	}

	void processFrame(const ZWave::FrameWrapper &frame);

	NetworkStats network_stats_;
	std::vector<NodeStats> nodes_;

private:
	bool shouldCheckHomeID()
	{
		return home_id_ != 0;
	}

	uint32_t home_id_;
};
