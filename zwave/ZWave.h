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

#include <set>
#include <vector>

#include <unirec/unirec.h>

namespace ZWave {

#define FRAME_MIN_SIZE_C12 10
#define FRAME_MIN_SIZE_C3 11

enum Channel : uint8_t {
	C1 = 1,
	C2 = 2,
	C3 = 3
};

enum CC : uint8_t {
	System = 0x01,
	Basic = 0x20,
	Configuration = 0x70,
	ManufacturerSpecific = 0x72,
	Version = 0x86,
	//...
};

enum CCSystemCommand : uint8_t {
	NodeInfo = 0x01,
	GetSupportedCC = 0x02,
	AssignNodeId = 0x03,
	DoNLTest = 0x04,
	GetNL = 0x05,
	ReportNL = 0x06,
	NLTestDone = 0x07,
	TransferPresentation = 0x08,
	SRCacheAssignment = 0x0C,
	BackboneCacheAssignment = 0x14,
	SRRequest = 0x15,
	NLTest = 0x18,
};

struct Transport_header {
	uint8_t home_id3;
	uint8_t home_id2;
	uint8_t home_id1;
	uint8_t home_id0;
	uint8_t src_id;
	struct {
		uint8_t header_type : 4;
		uint8_t speed : 1;
		uint8_t low_power : 1;
		uint8_t ack_req : 1;
		uint8_t routed : 1;
	} fc0;
	struct {
		uint8_t seq_number : 4;
		uint8_t reserved0 : 1;
		uint8_t beam_control : 2;
		uint8_t reserved1 : 1;
	} fc1;
	uint8_t length;
	uint8_t dst_id;
};

#define LENGTH_POS 7
#define NETWORK_OR_PAYLOAD_POS 9
#define NETWORK_HOPS_POS 11

struct Network_header {
// SR - Source Route
	struct {
		uint8_t sr_type : 4; // 0x00 App frame, 0x03 Route ACK, 0x05 Route NACK
		uint8_t failed_hop : 4; // index of failed hop
	};
	struct {
		uint8_t hop_index : 4; // index of dst hop
		uint8_t sr_len : 4; // length of hops
	};
	// uint8_t first_hop;
	// uint8_t hop2;
	// uint8_t hop3;
	// uint8_t hop4;
};

std::string commandClassValToStr(uint8_t commandClass);
std::string systemCommandValToStr(uint8_t systemCommand);

uint8_t checksum(const uint8_t *bytes, uint8_t length);
uint16_t crc16(const uint8_t *bytes, uint8_t length);

class FrameWrapper {
public:
	enum Type : uint8_t {
		Singlecast = 0x01,
		Multicast = 0x02,
		Ack = 0x03,
	};

	enum RoutedType : uint8_t {
		AppFrame = 0x00,
		RouteAck = 0x03,
		RouteNack = 0x05
	};

	FrameWrapper(
		const uint8_t *bytes,
		uint8_t size,
		Channel channel)
		: header_((Transport_header *) bytes),
		  bytes_(bytes),
		  size_(size),
		  channel_(channel)
	{
	}

	ur_time_t timestamp() const;
	void setTimestamp(ur_time_t timestamp);
	void setTimestampNow();

	// call this before other methods to know frame is valid
	bool isOK() const;
	bool sizeOK() const;
	bool checkSumOK() const;

	uint32_t homeId() const;
	uint8_t src() const;
	uint8_t dst() const;
	uint8_t length() const;
	uint8_t seqNumber() const;

	bool isSinglecast() const;
	bool isMulticast() const;
	bool isBroadcast() const;
	bool isRequest() const;
	bool isAck() const;
	bool isRouted() const;

	std::string headerTypeStr() const;

	// return payload position if everything is ok and payload is present
	//        length position if frame is not singlecast or ack
	//        networkHeaderPos if network header is of unknown format or invalid
	uint8_t payloadPos() const;
	uint8_t checksumPos() const;

	// NETWORK HEADER methods
	// !! first call isNetworkHeaderOK
	bool isNetworkHeaderOK() const;
	const Network_header *networkHeader() const;
	std::string routedTypeStr() const;
	uint8_t srcHopId() const;
	uint8_t dstHopId() const;
	uint8_t failedHopId() const;
	const uint8_t *networkHops() const;
	std::string constructRouteString() const;

	bool getCommandClassAndCommand(uint8_t &cc, uint8_t &command) const;

	bool getNodeIdsFromNL(std::set<uint8_t> &nodes) const;
	bool getHopsFromSRCacheEntry(std::vector<uint8_t> &hops) const;

	void print(bool parse = false) const;

private:
	void printNetworkHeader() const;
	void printPayload() const;

public:
	const Transport_header *header_;
	const uint8_t *const bytes_;
	const uint8_t size_;
	const Channel channel_;

private:
	ur_time_t timestamp_;

	mutable bool network_header_already_checked_ = false;
	mutable bool network_header_ok_ = false;
};

} //end of namespace ZWave
