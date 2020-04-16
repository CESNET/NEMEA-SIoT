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

#include <iostream>

#include "ZWaveStatistics.h"

using namespace ZWave;

void ZWaveStatistics::processFrame(const ZWave::FrameWrapper &frame)
{
	//corrupted frames
	if (!frame.isOK()) {
		network_stats_.corrupted_c++;

		switch (frame.channel_) {
			case ZWave::C1:
				network_stats_.corrupted_ch1_c++;
				break;
			case ZWave::C2:
				network_stats_.corrupted_ch2_c++;
				break;
			case ZWave::C3:
				network_stats_.corrupted_ch3_c++;
				break;
		}
		return;
	}

	//process just frames from our network
	if (shouldCheckHomeID() && frame.homeId() != home_id_) { return; }

	NodeStats &src_node = nodes_[frame.src()];
	NodeStats &dst_node = nodes_[frame.dst()];
	uint32_t timestamp = ur_time_get_sec(frame.timestamp());

	network_stats_.total_ok_c++;
	src_node.should_be_reported = true;
	src_node.src_total_c++;
	src_node.src_total_lm_t = timestamp;

	if (frame.isMulticast()) {
		network_stats_.multicast_c++;
		src_node.src_multicast_c++;
		return;
	}

	//drop unknown frames
	if (!frame.isSinglecast() && !frame.isAck()) { return; }

	if (frame.isBroadcast()) {
		network_stats_.broadcast_c++;
		src_node.src_broadcast_c++;
	}
	else { //singlecast or ack
		dst_node.should_be_reported = true;
		dst_node.dst_total_c++;
		dst_node.dst_total_lm_t = timestamp;

		if (frame.isAck()) {
			network_stats_.ack_c++;
			src_node.src_ack_c++;
			dst_node.dst_ack_c++;
		}
		else { //singlecast
			network_stats_.singlecast_c++;
			src_node.src_singl_c++;
			dst_node.dst_singl_c++;
			src_node.src_singl_lm_t = timestamp;
			dst_node.dst_singl_lm_t = timestamp;
		}
	}

	if (frame.isRouted()) {
		network_stats_.routed_c++;

		if (frame.isNetworkHeaderOK()) {
			auto net_header = frame.networkHeader();
			switch(net_header->sr_type) {
				case ZWave::FrameWrapper::AppFrame:
					network_stats_.routed_app_c++;
					break;
				case ZWave::FrameWrapper::RouteAck:
					network_stats_.routed_ack_c++;
					break;
				case ZWave::FrameWrapper::RouteNack:
					network_stats_.routed_nack_c++;
					src_node.src_nack_c++;
					dst_node.dst_nack_c++;
					NodeStats &failed_hop_node = nodes_[frame.failedHopId()];
					failed_hop_node.should_be_reported = true;
					failed_hop_node.failed_hop_nack_c++;
					break;
			}
		}
	}

	//Payload
	uint8_t begin = frame.payloadPos();
	uint8_t end = frame.checksumPos();
	uint8_t payloadLength = end - begin;
	//we need command class and command
	if (payloadLength < 2) { return; }

	uint8_t command_class = frame.bytes_[begin];
	uint8_t command = frame.bytes_[begin + 1];

	if (command_class != ZWave::CC::System) { return; }

	switch (command) {
		case CCSystemCommand::DoNLTest:
		case CCSystemCommand::GetNL:
		case CCSystemCommand::ReportNL:
		case CCSystemCommand::NLTestDone:
		case CCSystemCommand::SRCacheAssignment:
		case CCSystemCommand::BackboneCacheAssignment:
		case CCSystemCommand::SRRequest:
		case CCSystemCommand::NLTest: network_stats_.net_manag_c++;
	}
}
