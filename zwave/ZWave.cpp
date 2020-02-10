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

#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <sys/time.h>

#include "Conversion.h"
#include "ZWave.h"

using namespace std;
using namespace ZWave;

std::string ZWave::commandClassValToStr(uint8_t commandClass)
{
	static const std::unordered_map<uint8_t, std::string> ccMap {
		{ 0x00, "No Operation" },
		{ 0x01, "System" },

		{ 0x20, "Basic" },
		{ 0x21, "Controller Replication" },
		{ 0x22, "Application Status" },
		{ 0x23, "ZIP" },
		{ 0x24, "Security Panel Mode" },
		{ 0x25, "Switch Binary" },
		{ 0x26, "Switch Multilevel" },
		{ 0x27, "Switch All" },
		{ 0x28, "Switch Toggle Binary" },
		{ 0x29, "Switch Toggle Multilevel" },
		{ 0x2A, "Chimney Fan" },
		{ 0x2B, "Scene Activation" },
		{ 0x2C, "Scene Actuator Conf" },
		{ 0x2D, "Scene Controller Conf" },
		{ 0x2E, "Security Panel Zone" },
		{ 0x2F, "Security Panel Zone Sensor" },

		{ 0x30, "Sensor Binary" },
		{ 0x31, "Sensor Multilevel" },
		{ 0x32, "Meter" },
		{ 0x33, "Color Control" },
		{ 0x34, "Network Management Inclusion" },
		{ 0x35, "Meter Pulse" },
		{ 0x36, "Basic Tariff Info" },
		{ 0x37, "HRV Status" },
		{ 0x38, "Thermostat Heating" },
		{ 0x39, "HRV Control" },
		{ 0x3A, "DCP Config" },
		{ 0x3B, "DCP Monitor" },
		{ 0x3C, "Meter TBL Config" },
		{ 0x3D, "Meter TBL Monitor" },
		{ 0x3E, "Meter TBL Push" },
		{ 0x3F, "Prepayment" },

		{ 0x40, "Thermostat Mode" },
		{ 0x41, "Prepayment Encapsulation" },
		{ 0x42, "Thermostat Operating State" },
		{ 0x43, "Thermostat Setpoint" },
		{ 0x44, "Thermostat Fan Mode" },
		{ 0x45, "Thermostat Fan State" },
		{ 0x46, "Climate Control Schedule" },
		{ 0x47, "Thermostat Setback" },
		{ 0x48, "Rate TBL Config" },
		{ 0x49, "Rate TBL Monitor" },
		{ 0x4A, "Tariff Config" },
		{ 0x4B, "Tariff TBL Monitor" },
		{ 0x4C, "Door Lock Logging" },
		{ 0x4D, "Network Management Basic" },
		{ 0x4E, "Schedule Entry Lock" },
		{ 0x4F, "ZIP 6LOWPAN" },

		{ 0x50, "Basic Window Covering" },
		{ 0x51, "MTP Window Covering" },
		{ 0x52, "Network Management Proxy" },
		{ 0x53, "Schedule" },
		{ 0x54, "Network Management Primary" },
		{ 0x55, "Transport Service" },
		{ 0x56, "CRC16 Encap" },
		{ 0x57, "Application Capability" },
		{ 0x58, "ZIP ND" },
		{ 0x59, "Association GRP Info" },
		{ 0x5A, "Device Reset Locally" },
		{ 0x5B, "Central Scene" },
		{ 0x5C, "IP Association" },
		{ 0x5D, "Antitheft" },
		{ 0x5E, "ZWavePlus Info" },
		{ 0x5F, "ZIP Gateway" },

		{ 0x60, "Multi Channel" },
		{ 0x61, "ZIP Portal" },
		{ 0x62, "Door Lock" },
		{ 0x63, "User Code" },
//		{ 0x64, "" },
		{ 0x65, "DMX" },
		{ 0x66, "Barrier Operator" },
		{ 0x67, "Network Management Install" },
		{ 0x68, "ZIP Naming" },
		{ 0x69, "Mailbox" },
		{ 0x6A, "Window Covering" },
		{ 0x6B, "Irrigation" },
		{ 0x6C, "Supervision" },
//		{ 0x6D, "" },
//		{ 0x6E, "" },
//		{ 0x6F, "" },

		{ 0x70, "Configuration" },
		{ 0x71, "Alarm" },
		{ 0x72, "Manufacturer Specific" },
		{ 0x73, "Power Level" },
//		{ 0x74, "" },
		{ 0x75, "Protection" },
		{ 0x76, "Lock" },
		{ 0x77, "Node Naming" },
//		{ 0x78, "" },
//		{ 0x79, "" },
		{ 0x7A, "Firmware Update MD" },
		{ 0x7B, "Grouping Name" },
		{ 0x7C, "Remote Association Activate" },
		{ 0x7D, "Remote Association" },
//		{ 0x7E, "" },
//		{ 0x7F, "" },

		{ 0x80, "Battery" },
		{ 0x81, "Clock" },
		{ 0x82, "Hail" },
//		{ 0x83, "" },
		{ 0x84, "WakeUp" },
		{ 0x85, "Association" },
		{ 0x86, "Version" },
		{ 0x87, "Indicator" },
		{ 0x88, "Proprietary" },
		{ 0x89, "Language" },
		{ 0x8A, "Time" },
		{ 0x8B, "Time Parameters" },
		{ 0x8C, "" },
		{ 0x8D, "" },
		{ 0x8E, "Multi Channel Association" },
		{ 0x8F, "Multi Command" },

		{ 0x90, "Energy Production" },
		{ 0x91, "Manufacturer Proprietary" },
		{ 0x92, "Screen MD"	 },
		{ 0x93, "Screen Attributes" },
		{ 0x94, "Simple AV Control" },
		{ 0x95, "AV Content Directory MD" },
		{ 0x96, "AV Renderer Status" },
		{ 0x97, "AV Content Search MD" },
		{ 0x98, "Security V1" },
		{ 0x99, "AV Tagging MD" },
		{ 0x9A, "IP Configuration" },
		{ 0x9B, "Association Command Configuration" },
		{ 0x9C, "Sensor Alarm" },
		{ 0x9D, "Silence Alarm" },
		{ 0x9E, "Sensor Configuration" },
		{ 0x9F, "Security S2" },
	};

	const auto &it = ccMap.find(commandClass);
	if (it != ccMap.end())
		return it->second;

	return "Command Class Not Found";
}

std::string ZWave::systemCommandValToStr(uint8_t systemCommand)
{
	static const std::unordered_map<uint8_t, std::string> systemCMap {
		{ CCSystemCommand::NodeInfo,                "Node Info" },
		{ CCSystemCommand::GetSupportedCC,          "Get supported CC" },
		{ CCSystemCommand::AssignNodeId,            "Assign Id" },
		{ CCSystemCommand::DoNLTest,                "Do NL Test" },
		{ CCSystemCommand::GetNL,                   "Get NL" },
		{ CCSystemCommand::ReportNL,                "Report NL" },
		{ CCSystemCommand::NLTestDone,              "NL Test Done" },
		{ CCSystemCommand::TransferPresentation,    "Transfer Presentation" },// Listen mode
		{ CCSystemCommand::SRCacheAssignment,       "SR Cache Assignment" },
		{ CCSystemCommand::BackboneCacheAssignment, "Backbone Cache Assignment" },
		{ CCSystemCommand::SRRequest,               "SR Request" },
		{ CCSystemCommand::NLTest,                  "NL Test" },
	};

	const auto &it = systemCMap.find(systemCommand);
	if (it != systemCMap.end())
		return it->second;

	return "System Command Not Found";
}

//XOR checksum
uint8_t ZWave::checksum(const uint8_t *bytes, uint8_t length)
{
	uint8_t checksum = 0xFF;
	for (auto i = 0; i < length; ++i)
		checksum ^= bytes[i];

	return checksum;
}

//CRC-16/AUG-CCITT
uint16_t ZWave::crc16(const uint8_t *bytes, uint8_t length)
{
	uint8_t x;
	uint16_t crc = 0x1D0F;

	while (length--) {
		x = crc >> 8 ^ *bytes++;
		x ^= x >> 4;
		crc = (crc << 8) ^ ((uint16_t) (x << 12)) ^ ((uint16_t) (x << 5)) ^
			  ((uint16_t) x);
	}

	return crc;
}

ur_time_t FrameWrapper::timestamp() const
{
	return timestamp_;
}

void FrameWrapper::setTimestamp(ur_time_t timestamp)
{
	timestamp_ = timestamp;
}

void FrameWrapper::setTimestampNow()
{
	timeval tv = {};
	::gettimeofday(&tv, nullptr);
	timestamp_ = ur_time_from_sec_usec(tv.tv_sec, tv.tv_usec);
}

bool FrameWrapper::isOK() const
{
	return sizeOK() && checkSumOK();
}

bool FrameWrapper::sizeOK() const
{
	if (channel_ == Channel::C1 || channel_ == Channel::C2)
	{
		return size_ >= FRAME_MIN_SIZE_C12 && header_->length <= size_ && header_->length >= FRAME_MIN_SIZE_C12;
	}
	if (channel_ == Channel::C3)
	{
		return size_ >= FRAME_MIN_SIZE_C3 && header_->length <= size_ && header_->length >= FRAME_MIN_SIZE_C3;;
	}

	return false;
}

bool FrameWrapper::checkSumOK() const
{
	if (channel_ == C1 || channel_ == C2)
	{
		auto cs = checksum(bytes_, header_->length - 1);
		return cs == *(bytes_ + header_->length - 1);
	}
	if (channel_ == C3)
	{
		auto crc = crc16(bytes_, header_->length - 2);
		return crc == (*(bytes_ + header_->length - 2) << 8) | *(bytes_ + header_->length - 1);
	}

	return false;
}

uint32_t FrameWrapper::homeId() const
{
	return header_->home_id3 << 24 | header_->home_id2 << 16 | header_->home_id1 << 8 | header_->home_id0;
}

uint8_t FrameWrapper::src() const
{
	return header_->src_id;
}

uint8_t FrameWrapper::dst() const
{
	return header_->dst_id;
}

uint8_t FrameWrapper::length() const
{
	return header_->length;
}

uint8_t FrameWrapper::seqNumber() const
{
	return header_->fc1.seq_number;
}

bool FrameWrapper::isSinglecast() const
{
	return header_->fc0.header_type == Singlecast;
}

bool FrameWrapper::isMulticast() const
{
	return header_->fc0.header_type == Multicast;
}

bool FrameWrapper::isBroadcast() const
{
	return header_->fc0.header_type == Singlecast && header_->dst_id == 0xFF;
}

bool FrameWrapper::isRequest() const
{
	return header_->fc0.ack_req;
}

bool FrameWrapper::isAck() const
{
	return header_->fc0.header_type == Ack;
}

bool FrameWrapper::isRouted() const
{
	return header_->fc0.routed;
}

std::string FrameWrapper::headerTypeStr() const
{
	switch (header_->fc0.header_type) {
		case Type::Singlecast:
			if (header_->dst_id == 0xFF)
				return "Broadcast";
			else
				return "Singlecast";
		case Type::Multicast:
			return "Multicast";
		case Type::Ack:
			return "Ack";
		default:
			return to_string(header_->fc0.header_type);
	}
}

uint8_t FrameWrapper::payloadPos() const
{
	if (!isSinglecast() && !isAck()) //TODO ble
		return LENGTH_POS;

	if (!isRouted() || !isNetworkHeaderOK())
		return NETWORK_OR_PAYLOAD_POS;

	return NETWORK_HOPS_POS + networkHeader()->sr_len;
};

uint8_t FrameWrapper::checksumPos() const
{
	return header_->length - (channel_ == Channel::C3 ? 2 : 1);
};

bool FrameWrapper::isNetworkHeaderOK() const
{
	uint8_t csPos = checksumPos();

	if (NETWORK_HOPS_POS >= csPos)
		return false;

	auto header = networkHeader();
	if (NETWORK_HOPS_POS + header->sr_len >= csPos)
		return false;

	if (header->sr_type != AppFrame && header->sr_type != RouteAck && header->sr_type != RouteNack)
		return false;

	if (header->sr_len > 4)
		return false;

	if (header->failed_hop > 5)
		return false;

	return (header->hop_index == 0xF) || (header->hop_index >= 0 && header->hop_index < header->sr_len);
};

const Network_header *FrameWrapper::networkHeader() const
{
	return (Network_header *) (bytes_ + NETWORK_OR_PAYLOAD_POS);
}

std::string FrameWrapper::routedTypeStr() const
{
	auto header = networkHeader();
	switch(header->sr_type) {
		case AppFrame:
			return "Request";
		case RouteAck:
			return "Ack";
		case RouteNack:
			return "Nack";
		default:
			return to_string(header->sr_type);
	}
}

uint8_t FrameWrapper::srcHopId() const
{
	auto header = networkHeader();

	if (header->sr_type == AppFrame)
	{
		if (header->hop_index == 0)
			return header_->src_id;

		return bytes_[NETWORK_HOPS_POS + header->hop_index - 1];
	}
	else
	{
		if (header->hop_index == 0xF)
			return bytes_[NETWORK_HOPS_POS];

		if (header->hop_index + 1 == header->sr_len)
			return header_->dst_id;

		return bytes_[NETWORK_HOPS_POS + header->hop_index + 1];
	}
}

uint8_t FrameWrapper::dstHopId() const
{
	auto header = networkHeader();

	if (header->sr_type == AppFrame)
	{
		if (header->hop_index == header->sr_len)
			return header_->dst_id;

		return bytes_[NETWORK_HOPS_POS + header->hop_index];
	}
	else
	{
		if (header->hop_index == 0xF)
			return header_->src_id;

		return bytes_[NETWORK_HOPS_POS + header->hop_index];
	}
}

uint8_t FrameWrapper::failedHopId() const
{
	auto header = networkHeader();

	if (header->failed_hop == 0)
		return 0;

	if (header->failed_hop <= header->sr_len)
		return bytes_[NETWORK_HOPS_POS + header->failed_hop - 1];

	return header_->dst_id;
}

const uint8_t *FrameWrapper::networkHops() const
{
	return bytes_ + NETWORK_HOPS_POS;
}

std::string FrameWrapper::constructRouteString() const
{
	auto header = networkHeader();

	std::stringstream ss;

	ss << std::hex << setfill('0') << setw(2) << (int) src();
	ss << "->";
	for (uint8_t i = 0; i < header->sr_len; ++i)
	{
		ss << std::hex << setfill('0') << setw(2) << (int) *(networkHops() + i);
		ss << "->";
	}
	ss << std::hex << setfill('0') << setw(2) << (int) dst();

	return ss.str();
}

bool FrameWrapper::getCommandClassAndCommand(uint8_t &cc, uint8_t &command) const
{
	const uint8_t payload_begin = payloadPos();
	const uint8_t payload_len = checksumPos() - payload_begin;

	if (payload_len < 2) { return false; }

	cc = bytes_[payload_begin];
	command = bytes_[payload_begin + 1];

	return true;
}

bool FrameWrapper::getNodeIdsFromNL(std::set<uint8_t> &nodes) const
{
	nodes.clear();

	const uint8_t payload_begin = payloadPos();
	const uint8_t payload_len = checksumPos() - payload_begin;

	if (payload_len < 4) { return false; }

	const uint8_t nl_bitfield_len = bytes_[payload_begin + 2];
	const uint8_t exp_payload_len = (uint8_t) (3 + nl_bitfield_len);

	if (payload_len < exp_payload_len) { return false; }

	for (uint8_t i = 0; i < nl_bitfield_len; ++i)
	{
		const uint8_t byte = bytes_[payload_begin + 3 + i];
		for (uint8_t j = 0; j < 8; ++j)
		{
			const uint8_t node_id = (uint8_t) (8 * i + j + 1);

			const bool is_set = (bool)((byte >> j) & 0b00000001);

			if (is_set) { nodes.insert(node_id); }
		}
	}

	return true;
}

bool FrameWrapper::getHopsFromSRCacheEntry(std::vector<uint8_t> &hops) const
{
	hops.clear();

	const uint8_t payload_begin = payloadPos();
	const uint8_t payload_len = checksumPos() - payload_begin;

	if (payload_len < 6) { return false; }

	const uint8_t sr_len = (uint8_t) (bytes_[payload_begin + 3] & 0b00001111);
	const uint8_t exp_payload_len = (uint8_t) (5 + sr_len);

	if (payload_len < exp_payload_len) { return false; }

	for (uint8_t i = 0; i < sr_len; ++i)
	{
		hops.push_back(bytes_[payload_begin + 4 + i]);
	}

	return true;
}

void FrameWrapper::print(bool parse) const
{
	if (!sizeOK())
	{
		cout << endl
			 << "\033[1;31m"
			 << "Channel " << to_string(channel_)
			 << ", frame corrupted: bad size"
			 << "\033[0m"
			 << endl;

		for (auto i = 0; i < size_; ++i)
			cout << hex << setfill('0') << setw(2) << (int) bytes_[i] << " ";
		cout << endl;

		return;
	}

	if (!checkSumOK())
	{
		cout << endl
			 << "\033[1;33m"
			 << "Channel " << to_string(channel_)
			 << ", frame corrupted: bad checksum"
			 << "\033[0m"
			 << endl;

		for (auto i = 0; i < size_; ++i)
			cout << hex << setfill('0') << setw(2) << (int) bytes_[i] << " ";
		cout << endl;

		return;
	}

	cout << endl
		 << "\033[1;32m"
		 << "Channel " << to_string(channel_)
		 << ", frame OK"
		 << "\033[0m"
		 << endl;


	for (auto i = 0; i < size_; ++i)
		cout << hex << setfill('0') << setw(2) << (int) bytes_[i] << " ";
	cout << endl;

	if (!parse) { return; }

	cout << "============================================================" << endl;
	cout << "HomeID: " << (int) header_->home_id3
		 << " " << (int) header_->home_id2
		 << " " << (int) header_->home_id1
		 << " " << (int) header_->home_id0 << endl;
	cout << "Source NodeID: " << (int) header_->src_id << endl;
	cout << "Destination NodeID: " << (int) header_->dst_id << endl;
	cout << "------------------------------------------------------------" << endl;
	cout << "Header Type: " << (int) header_->fc0.header_type << endl;
	cout << "Speed: " << (int) header_->fc0.speed << endl;
	cout << "Low Power: " << (int) header_->fc0.low_power << endl;
	cout << "Ack/Req: " << (int) header_->fc0.ack_req << endl;
	cout << "Routed: " << (int) header_->fc0.routed << endl;
	cout << "Sequence Number: " << (int) header_->fc1.seq_number << endl;
	cout << "Beam Control: " << (int) header_->fc1.beam_control << endl;
	cout << "------------------------------------------------------------" << endl;

	if (isRouted())
		printNetworkHeader();

	if (!isAck())
		printPayload();

	cout << "============================================================" << endl;
}

void FrameWrapper::printNetworkHeader() const
{
	cout << "NETWORK HEADER:" << endl;

	if (!isNetworkHeaderOK())
		return;

	auto header = networkHeader();

	cout << "SR Type: " << (int) header->sr_type << endl;
	cout << "Failed Hop: " << (int) header->failed_hop << endl;
	cout << "SR Length: " << (int) header->sr_len << endl;
	cout << "Hop Index: " << (int) header->hop_index << endl;

	cout << "Hops:";

	uint8_t begin = NETWORK_HOPS_POS;
	uint8_t end = begin + header->sr_len;

	for (uint8_t i = begin; i < end; ++i) {
		cout << " " << (int) bytes_[i];
	}
	cout << endl;

	cout << "------------------------------------------------------------" << endl;
}

void FrameWrapper::printPayload() const
{
	cout << "PAYLOAD: ";

	uint8_t begin = payloadPos();
	uint8_t end = checksumPos();
	if (begin == end)
		return;

	for (auto i = begin; i < end; ++i)
		cout << " " << hex << setfill('0') << setw(2) << (int) bytes_[i];
	cout << endl;

	cout << "CommandClass: " << commandClassValToStr(bytes_[begin]) << endl;

	if (begin + 1 == end)
		return;

	cout << "Command: ";
	if (bytes_[begin] == 0x01)
		cout << systemCommandValToStr(bytes_[begin + 1]);
	else
		cout << (int) bytes_[begin + 1];
	cout << endl;

}
