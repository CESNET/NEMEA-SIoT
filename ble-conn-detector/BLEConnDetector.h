#ifndef BLE_CONN_DETECTOR_H
#define BLE_CONN_DETECTOR_H

#include <iostream>
#include <unordered_map>
#include <unirec/unirec.h>

#include "BLEConnModel.h"
#include "SimpleStatisticModel.h"

typedef struct {
	ur_time_t  timestamp;
	mac_addr_t bdaddr;
	uint8_t    bdaddr_type; // Type of BDADDR: 0x00 = Public, 0x01 = Random
	int8_t     rssi;
} adv_report;

/* Events (Exceptions) */
struct ConnectionFound
{
public:
	const ur_time_t  timestamp;
	const mac_addr_t bdaddr;
	const uint8_t    bdaddr_type;
	const uint32_t   usage_duration;

	ConnectionFound(ur_time_t timestamp, mac_addr_t bdaddr, uint8_t bdaddr_type, uint32_t usage_duration)
		: timestamp(timestamp)
			, bdaddr(bdaddr)
			, bdaddr_type(bdaddr_type)
			, usage_duration(usage_duration)
		{}

};

inline uint64_t convertMacToID(const mac_addr_t *mac) {
	return
		uint64_t(mac->bytes[0]) << 40 |
		uint64_t(mac->bytes[1]) << 32 |
		uint64_t(mac->bytes[2]) << 24 |
		uint64_t(mac->bytes[3]) << 16 |
		uint64_t(mac->bytes[4]) <<  8 |
		uint64_t(mac->bytes[5]);
}

/* Main detector class */
class BLEConnDetector
{
public:
	BLEConnDetector(void) {}

	~BLEConnDetector(void) {
		for (auto& it : models) {
			if (it.second != NULL)
				delete it.second;
		}
		models.clear();
	}

	void processAdvReport(const adv_report *report) {

		uint64_t id;
		char     buf[MAC_STR_LEN];

		id = convertMacToID(&report->bdaddr);

		mac_to_str(&report->bdaddr, buf);

		auto elem = models.find(id);
		if (elem == models.end()) {
			models[id] = new SimpleStatisticModel();
			std::cout << "Created model for device " << buf << std::endl;
		} else {
			auto model = elem->second;
			try {
				model->receivedAdvAt(report->timestamp);
			} catch (SSMInitialised evt) {
				std::cout << "Model for " << buf << " initialised." << std::endl;
				std::cout << "\t(Midpoint: " << evt.midpoint << "us;";
				std::cout << " Threshold: " << evt.threshold << "us)" << std::endl;
			} catch (ConnectionDetected evt) {
				throw ConnectionFound(evt.timestamp, report->bdaddr, report->bdaddr_type, (uint32_t) evt.duration);
			}
		}

	}

private:
	std::unordered_map<uint64_t, BLEConnModel*> models;
};

#endif

/* vim: set ts=3 sw=3 noet */
