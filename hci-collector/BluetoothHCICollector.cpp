/**
 * @file BluetoothHCICollector.cpp
 * @brief Collect all Bluetooth packets from specified HCI and send them to the output.
 * @author Jozef Halaj <xhalaj03@stud.fit.vutbr.cz>
 * @date 2018
 */

#include <csignal>
#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <libtrap/trap.h>
#include <unirec/unirec.h>

#include "fields.h"

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
		perror("Error: failed to open HCI socket.");
		return -1;
	}

	hci_filter_clear(&flt);
	hci_filter_all_ptypes(&flt);
	hci_filter_all_events(&flt);

	if (setsockopt(fd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
		perror("Error: failed to set HCI filter.");
		close(fd);
		return -1;
	}

	if (setsockopt(fd, SOL_HCI, HCI_DATA_DIR, &opt, sizeof(opt)) < 0) {
		perror("Error: failed to enable HCI data direction info.");
		close(fd);
		return -1;
	}

	if (setsockopt(fd, SOL_HCI, HCI_TIME_STAMP, &opt, sizeof(opt)) < 0) {
		perror("Error: failed to enable HCI time stamps.");
		close(fd);
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = dev;
	addr.hci_channel = HCI_CHANNEL_RAW;

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Error: failed to bind HCI socket.");
		close(fd);
		return -1;
	}

	return fd;
}

int exportPackets(int fd, const mac_addr_t &hci_dev_mac, ur_template_t *out_template, void *out_record)
{
	vector<unsigned char> buffer(HCI_MAX_FRAME_SIZE);
	vector<unsigned char> control(64);
	struct pollfd pollFd;
	int dir;
	struct timeval tv;

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

			perror("receive failed");
			return -1;
		}

		for (cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_level != SOL_HCI)
				continue;

			switch (cmsg->cmsg_type) {
			case HCI_CMSG_DIR:
				memcpy(&dir, CMSG_DATA(cmsg), sizeof(dir));
				break;
			case HCI_CMSG_TSTAMP:
				memcpy(&tv, CMSG_DATA(cmsg), sizeof(tv));
				break;
			}
		}

		ur_time_t timestamp = ur_time_from_sec_msec(tv.tv_sec, tv.tv_usec / 1000);

		ur_set(out_template, out_record, F_HCI_DEV_MAC, hci_dev_mac);
		ur_set(out_template, out_record, F_TIMESTAMP, timestamp);
		ur_set(out_template, out_record, F_DATA_DIRECTION, dir);
		ur_set(out_template, out_record, F_PACKET_TYPE, buffer[0]);
		ur_set_var(out_template, out_record, F_PACKET, buffer.data() + 1, len - 1);

		trap_send(0, out_record, ur_rec_size(out_template, out_record));
	}
}

int main(int argc, char *argv[])
{
	int exit_value = 0;
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
			return exit_value;
		}
	}

	out_template = ur_create_output_template(0, "HCI_DEV_MAC, TIMESTAMP, DATA_DIRECTION, PACKET_TYPE, PACKET", NULL);
	if (out_template == NULL) {
		cerr << "Error: output template could not be created." << endl;
		exit_value = -1;
		goto cleanup;
	}

	out_record = ur_create_record(out_template, 0);
	if (out_record == NULL) {
		cerr << "Error: Memory allocation problem (output record).";
		exit_value = -1;
		goto cleanup;
	}

	socket = openHCISocket(hci_dev);
	if (socket == -1) {
		exit_value = -1;
		goto cleanup;
	}

	//Get MAC address of the HCI device
	hci_dev_info info;
	memset(&info, 0, sizeof(info));
	info.dev_id = hci_dev;

	if (ioctl(socket, HCIGETDEVINFO, &info) < 0) {
		cerr << "Error: Failed to get hci " + to_string(hci_dev) + " device info." << endl;
		exit_value = -1;
		goto cleanup;
	}

	bdaddr_t hci_dev_mac;
	baswap(&hci_dev_mac, &info.bdaddr);

	exit_value = exportPackets(socket, mac_from_bytes(hci_dev_mac.b), out_template, out_record);

cleanup:
	close(socket);

	ur_free_record(out_record);
	ur_free_template(out_template);

	TRAP_DEFAULT_FINALIZATION();
	FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)

	return exit_value;
}
