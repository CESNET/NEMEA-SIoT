/**
 * @file BluetoothHCICollector.cpp
 * @brief Collect all Bluetooth packets from specified HCI and send them to the output.
 * @author Jozef Halaj <xhalaj03@stud.fit.vutbr.cz>
 * @date 2018
 */

/*
 * Copyright (C) 2018 CESNET
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
#include <unordered_map>
#include <string>
#include <vector>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <libtrap/trap.h>
#include <unirec/unirec.h>

#include "fields.h"
#include "Conversion.h"

UR_FIELDS (
	time    TIMESTAMP
	macaddr DEV_ADDR
	macaddr HCI_DEV_ADDR
	uint8   PACKET_TYPE
	uint8   DATA_DIRECTION
	uint16  SIZE
	bytes   PACKET
)

#define UNIREC_TEMPLATE "TIMESTAMP, DEV_ADDR, HCI_DEV_ADDR, PACKET_TYPE, DATA_DIRECTION, SIZE, PACKET"

#define EVENT_STATUS_SUCCESS 0x00

using namespace std;

trap_module_info_t *module_info = NULL;

#define MODULE_BASIC_INFO(BASIC) \
	BASIC("bluetooth-hci-collector", \
		"Collect all Bluetooth packets from specified HCI and send them out.", 0, 1)

#define MODULE_PARAMS(PARAM) \
	PARAM('d', "dev", "HCI device id for packets collecting.", required_argument, "uint16")

static int g_stop = 0;

TRAP_DEFAULT_SIGNAL_HANDLER(g_stop = 1)

int openHCISocket(uint16_t dev)
{
	struct sockaddr_hci addr;
	struct hci_filter flt;
	int fd, opt = 1;

	fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
	if (fd < 0) {
		std::cerr << "Error: failed to open HCI socket. (" << errno << ")" << std::endl;
		return -1;
	}

	hci_filter_clear(&flt);
	hci_filter_all_ptypes(&flt);
	hci_filter_all_events(&flt);

	if (setsockopt(fd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
		std::cerr << "Error: failed to set HCI filter." << " (" << errno << ")" << std::endl;
		close(fd);
		return -1;
	}

	if (setsockopt(fd, SOL_HCI, HCI_DATA_DIR, &opt, sizeof(opt)) < 0) {
		std::cerr << "Error: failed to enable HCI data direction info." << " (" << errno << ")" << std::endl;
		close(fd);
		return -1;
	}

	if (setsockopt(fd, SOL_HCI, HCI_TIME_STAMP, &opt, sizeof(opt)) < 0) {
		std::cerr << "Error: failed to enable HCI time stamps." << " (" << errno << ")" << std::endl;
		close(fd);
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = dev;
	addr.hci_channel = HCI_CHANNEL_RAW;

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		std::cerr << "Error: failed to bind HCI socket." << " (" << errno << ")" << std::endl;
		close(fd);
		return -1;
	}

	return fd;
}

mac_addr_t findDevAddr(
	const uint8_t packet_type,
	const uint8_t* packet,
	const ssize_t size,
	unordered_map<uint16_t, mac_addr_t> &connections)
{
	switch (packet_type) {
	case HCI_EVENT_PKT: {
		if (size < 2) { //event header
			cerr << "error: invalid size of packet" << endl;
			break;
		}

		const hci_event_hdr *hdr = (hci_event_hdr *) packet;
		if (size - 2 != hdr->plen) {
			cerr << "error: invalid size of packet" << endl;
			break;
		}

		if (hdr->evt == EVT_LE_META_EVENT) {
			auto *event = (evt_le_meta_event *) (packet + 2);
			if (event->subevent == EVT_LE_CONN_COMPLETE) {
				if (hdr->plen != EVT_LE_META_EVENT_SIZE + EVT_LE_CONN_COMPLETE_SIZE) {
					cerr << "error: invalid size of packet" << endl;
					break;
				}

				auto *event_le = (evt_le_connection_complete *) event->data;
				if (event_le->status != EVENT_STATUS_SUCCESS) {
					break;
				}

				uint16_t handle = htobl(event_le->handle);

				bdaddr_t dev_mac;
				baswap(&dev_mac, &event_le->peer_bdaddr);

				connections[handle] = mac_from_bytes(dev_mac.b);
			}
		}
		else if (hdr->evt == EVT_DISCONN_COMPLETE) {
			if (hdr->plen != EVT_DISCONN_COMPLETE_SIZE) {
				cerr << "error: invalid size of packet" << endl;
				break;
			}

			auto *event = (evt_disconn_complete *) (packet + 2);

			uint16_t handle = htobl(event->handle);

			connections.erase(handle);
		}

		break;
	}
	case HCI_ACLDATA_PKT: {
		if (size < 8) { //acl header + l2cap header
			cerr << "error: invalid size of packet" << endl;
			break;
		}

		auto *hdr = (hci_acl_hdr *) packet;
		if (size - 4 != hdr->dlen) {
			cerr << "error: invalid size of packet" << endl;
			break;
		}

		uint16_t handle = htobl(hdr->handle) & 0x0FFF; // first 4 bits are ACL flags

		auto it = connections.find(handle);
		if (it == connections.end()) {
			break;
		}

		return it->second;
	}
	default: {
		break;
	}
	}

	mac_addr_t dev_addr = {0};
	return dev_addr;
}

int exportPackets(
	int fd,
	const mac_addr_t &hci_dev_addr,
	ur_template_t *out_template,
	void *out_record)
{
	unordered_map<uint16_t, mac_addr_t> connections;

	vector<unsigned char> buffer(HCI_MAX_FRAME_SIZE);
	vector<unsigned char> control(64);
	struct pollfd pollFd;
	int dir;

	pollFd.fd = fd;
	pollFd.events = POLLIN;
	pollFd.revents = 0;

	while (!g_stop) {
		int n = ::poll(&pollFd, 1, 1000); //timeout 1000ms
		if (n <= 0)
			continue;

		struct iovec iov;
		iov.iov_base = buffer.data();
		iov.iov_len = buffer.size();

		struct msghdr msg;
		memset(&msg, 0, sizeof(msg));
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		msg.msg_control = control.data();
		msg.msg_controllen = sizeof(control);

		ssize_t len = recvmsg(fd, &msg, MSG_DONTWAIT);
		if (len <= 0) {
			if (errno == EAGAIN)
				continue;

			std::cerr << "receive failed" << " (" << std::string(strerror(errno)) << ")" << std::endl;
			return 1;
		}

		for (cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_level != SOL_HCI)
				continue;

			switch (cmsg->cmsg_type) {
			case HCI_CMSG_DIR:
				memcpy(&dir, CMSG_DATA(cmsg), sizeof(dir));
				break;
			}
		}

		struct timeval tv;
		gettimeofday(&tv, NULL);
		ur_time_t timestamp = ur_time_from_sec_msec(tv.tv_sec, tv.tv_usec / 1000);

		mac_addr_t dev_addr = findDevAddr(buffer[0], buffer.data() + 1, len - 1, connections);

		ur_set(out_template, out_record, F_TIMESTAMP, timestamp);
		ur_set(out_template, out_record, F_DEV_ADDR, dev_addr);
		ur_set(out_template, out_record, F_HCI_DEV_ADDR, hci_dev_addr);
		ur_set(out_template, out_record, F_DATA_DIRECTION, dir);
		ur_set(out_template, out_record, F_PACKET_TYPE, buffer[0]);
		ur_set(out_template, out_record, F_SIZE, len - 1);
		ur_set_var(out_template, out_record, F_PACKET, buffer.data() + 1, len - 1);

		trap_send(0, out_record, ur_rec_size(out_template, out_record));
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int socket = -1;
	uint16_t hci_dev = 0;
	int opt;
	ur_template_t *out_template = NULL;
	void *out_record = NULL;

	INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
	TRAP_DEFAULT_INITIALIZATION(argc, argv, *module_info);

	TRAP_REGISTER_DEFAULT_SIGNAL_HANDLER();

	while ((opt = getopt_long(argc, argv, module_getopt_string, long_options, NULL)) != -1) {
		switch (opt) {
		case 'd':
			hci_dev = atoi(optarg);
			break;
		default:
			cerr << "Invalid argument." << endl;
			TRAP_DEFAULT_FINALIZATION();
			FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
			return 1;
		}
	}

	auto cleanup = [&](){
		TRAP_DEFAULT_FINALIZATION();

		if (out_template != NULL) { ur_free_template(out_template); }
		if (out_record != NULL) { ur_free_record(out_record); }

		if (socket != -1) { close(socket); }

		ur_finalize();

		FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
	};

	out_template = ur_create_output_template(0, UNIREC_TEMPLATE, NULL);
	if (out_template == NULL) {
		cerr << "Error: output template could not be created." << endl;
		cleanup();
		return 1;
	}

	out_record = ur_create_record(out_template, UR_MAX_SIZE);
	if (out_record == NULL) {
		cerr << "Error: Memory allocation problem (output record).";
		cleanup();
		return 1;
	}

	socket = openHCISocket(hci_dev);
	if (socket == -1) {
		cleanup();
		return 1;
	}

	//Get MAC address of the HCI device
	hci_dev_info info;
	memset(&info, 0, sizeof(info));
	info.dev_id = hci_dev;

	if (ioctl(socket, HCIGETDEVINFO, &info) < 0) {
		cerr << "Error: Failed to get hci " + to_string(hci_dev) + " device info." << endl;
		cleanup();
		return 1;
	}

	bdaddr_t hci_dev_addr;
	baswap(&hci_dev_addr, &info.bdaddr);

	int ret = exportPackets(socket, mac_from_bytes(hci_dev_addr.b), out_template, out_record);

	cleanup();
	return ret;
}
