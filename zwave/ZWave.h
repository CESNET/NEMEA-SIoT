#pragma once

#include <string>
#include <sys/time.h>
#include <unirec/unirec.h>

namespace ZWave {

#define FRAME_MIN_SIZE_C12 10
#define FRAME_MIN_SIZE_C3 11

enum Channel : uint8_t {
	C1 = 1,
	C2 = 2,
	C3 = 3
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

	ur_time_t timestamp() const
	{
		return timestamp_;
	}

	void setTimestamp(ur_time_t timestamp)
	{
		timestamp_ = timestamp;
	}

	void setTimestampNow()
	{
		timeval tv = {};
		::gettimeofday(&tv, nullptr);
		timestamp_ = ur_time_from_sec_msec(tv.tv_sec, tv.tv_usec / 1000);
	}

	bool isOK() const;
	bool sizeOK() const;
	bool checkSumOK() const;

	uint32_t homeId() const
	{
		return header_->home_id3 << 24 | header_->home_id2 << 16 | header_->home_id1 << 8 | header_->home_id0;
	}

	uint8_t src() const
	{
		return header_->src_id;
	}

	uint8_t dst() const
	{
		return header_->dst_id;
	}

	uint8_t length() const
	{
		return header_->length;
	}

	bool isSinglecast() const
	{
		return header_->fc0.header_type == Singlecast;
	}

	bool isMulticast() const
	{
		return header_->fc0.header_type == Multicast;
	}

	bool isBroadcast() const
	{
		return header_->fc0.header_type == Singlecast && header_->dst_id == 0xFF;
	}

	bool isRequest() const
	{
		return header_->fc0.ack_req;
	}

	bool isAck() const
	{
		return header_->fc0.header_type == Ack;
	}

	bool isRouted() const
	{
		return header_->fc0.routed;
	}

	uint8_t seqNumber() const
	{
		return header_->fc1.seq_number;
	}

	uint8_t payloadPos() const
	{
		if (!isSinglecast() && !isAck()) //TODO ble
			return LENGTH_POS;

		if (!isRouted() || !isNetworkHeaderOK())
			return NETWORK_OR_PAYLOAD_POS;

		return NETWORK_HOPS_POS + networkHeader()->sr_len;
	};

	uint8_t checksumPos() const
	{
		return header_->length - (channel_ == Channel::C3 ? 2 : 1);
	};

	bool isNetworkHeaderOK() const
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

	const Network_header *networkHeader() const
	{
		return (Network_header *) (bytes_ + NETWORK_OR_PAYLOAD_POS);
	}

	const uint8_t *networkHops() const
	{
		return bytes_ + NETWORK_HOPS_POS;
	}

	std::string routedTypeStr() const
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
				return std::to_string(header->sr_type);
		}
	}

	uint8_t srcHopId() const
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

	uint8_t dstHopId() const
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

	uint8_t failedHopId() const
	{
		auto header = networkHeader();

		if (header->failed_hop == 0)
			return 0;

		if (header->failed_hop <= header->sr_len)
			return bytes_[NETWORK_HOPS_POS + header->failed_hop - 1];

		return header_->dst_id;
	}

	std::string headerTypeStr() const
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
			return std::to_string(header_->fc0.header_type);
		}
	}

	void print(bool parse = false) const;

private:
	void printNetworkHeader() const;
	void printPayload() const;

public:
	const Transport_header *header_;
	const uint8_t *bytes_;
	const uint8_t size_;
	const Channel channel_;

private:
	ur_time_t timestamp_;
};

} //end of namespace ZWave
