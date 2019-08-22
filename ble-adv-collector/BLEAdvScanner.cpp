
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <errno.h>
#include <iomanip> // TODO: remove
#include <iostream>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "BLEAdvScanner.h"

BLEAdvScanner::BLEAdvScanner(void)
{
	hciDevID = hci_get_route(NULL);
	if (hciDevID < 0) {
		throw std::runtime_error("No Bluetooth device found.");
	}

	this->openHCISocket();
	this->running = true;
}

BLEAdvScanner::~BLEAdvScanner(void)
{
	close(sock);
}

void BLEAdvScanner::openHCISocket(void)
{
	struct sockaddr_hci addr;
	struct hci_dev_info info;
	struct hci_filter   filt;
	int                 enable = 1;	

	// SOCK_CLOEXEC = Security feature, socket will ble closed on the use of exec functions
	sock = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
	if (sock < 0) {
		throw std::runtime_error("Failed to open HCI socket.");
	}

	// Discover BD Address
	memset(&info, 0, sizeof(info));
	info.dev_id = hciDevID;
	
	if (ioctl(sock, HCIGETDEVINFO, (void *) &info) < 0) {
		throw std::runtime_error("Failed to get bluetooth address.");
	}

	bacpy(&bdaddr, &info.bdaddr);

	// Setup filter 
	hci_filter_clear(&filt);
	hci_filter_all_ptypes(&filt);
	hci_filter_all_events(&filt);

	if (setsockopt(sock, SOL_HCI, HCI_FILTER, &filt, sizeof(filt)) < 0) {
		throw std::runtime_error("Failed to set HCI filter.");
	}

	// Add time info to the HCI messages (1 = enable)
	if (setsockopt(sock, SOL_HCI, HCI_TIME_STAMP, &enable, sizeof(enable)) < 0) {
		throw std::runtime_error("Failed to enable HCI timestamps.");
	}

	// Bind the socket
	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = hciDevID;
	addr.hci_channel = HCI_CHANNEL_RAW; // Since kernel 3.4 only RAW channel for RAW sockets is allowed

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(sock);
		throw std::runtime_error("Failed to bind HCI socket.");
	}

}

const uint16_t BLEAdvScanner::getHCIDevice(void)
{
	return hciDevID;
}

const bdaddr_t* BLEAdvScanner::getBDAddr(void)
{
	return &bdaddr;
}

void BLEAdvScanner::setPassiveMode(void)
{
	struct hci_request req;
	le_set_scan_parameters_cp params;
	uint8_t status; // Response is a single status byte
	int err;

	memset(&params, 0, LE_SET_SCAN_PARAMETERS_CP_SIZE);
	params.type            = 0x00; // Passive scanning
	params.interval        = htobs(0x0010); // Minimal interval between scans => 10ms
	params.window          = htobs(0x0010); // The duration of actual scanning in one interval
	params.own_bdaddr_type = 0x00; // Public address
	params.filter          = 0x00; // Accept all ADV packets except directed ADV for different devices
	                      // 0x02 Accept all ADV packets except where initiator's identity address
	                      //      does not address this device

	
	memset(&req, 0, sizeof(hci_request));
	req.ogf = OGF_LE_CTL;
	req.ocf = OCF_LE_SET_SCAN_PARAMETERS;
	req.cparam = &params;
	req.clen = LE_SET_SCAN_PARAMETERS_CP_SIZE;
	req.rparam = &status;
	req.rlen = sizeof(status);
	req.event = EVT_CMD_COMPLETE;

	err = hci_send_req(sock, &req, 0);
	if (err != 0)
		throw std::runtime_error("Failed to set up passive mode.");
	
	if (status != 0)
		throw std::runtime_error("LE_Set_Scan_Parameters command failed.");
}

void BLEAdvScanner::start(bool filter_dup)
{
	struct hci_request req;
	le_set_scan_enable_cp params;
	uint8_t status; // Response is a single status byte
	int err;

	memset(&params, 0, LE_SET_SCAN_ENABLE_CP_SIZE);
	params.enable = true;
	params.filter_dup = filter_dup;
	
	memset(&req, 0, sizeof(hci_request));
	req.ogf = OGF_LE_CTL;
	req.ocf = OCF_LE_SET_SCAN_ENABLE;
	req.cparam = &params;
	req.clen = LE_SET_SCAN_ENABLE_CP_SIZE;
	req.rparam = &status;
	req.rlen = sizeof(status);
	req.event = EVT_CMD_COMPLETE;

	err = hci_send_req(sock, &req, 0);
	if (err != 0)
		throw std::runtime_error("Failed to start scanning.");
	
	if (status != 0)
		throw std::runtime_error("LE_Set_Scan_Enable command failed.");
	
	this->running = true;
}

void BLEAdvScanner::stop(void)
{
	struct hci_request req;
	le_set_scan_enable_cp params;
	uint8_t status; // Response is a single status byte
	int err;

	memset(&params, 0, LE_SET_SCAN_ENABLE_CP_SIZE);
	params.enable = false;
	params.filter_dup = true; // not needed, but nice to set the default
	
	memset(&req, 0, sizeof(hci_request));
	req.ogf = OGF_LE_CTL;
	req.ocf = OCF_LE_SET_SCAN_ENABLE;
	req.cparam = &params;
	req.clen = LE_SET_SCAN_ENABLE_CP_SIZE;
	req.rparam = &status;
	req.rlen = sizeof(status);
	req.event = EVT_CMD_COMPLETE;

	err = hci_send_req(sock, &req, 0);
	if (err != 0)
		throw std::runtime_error("Failed to stop scanning.");
	
	if (status != 0)
		throw std::runtime_error("LE_Set_Scan_Enable(0) command failed.");
	
	this->running = false;
}

adv_report BLEAdvScanner::scan(void)
{
	uint8_t buf[HCI_MAX_EVENT_SIZE];
	adv_report report;
	
	memset(&report, 0, sizeof(adv_report));

	while (read(sock, buf, HCI_MAX_EVENT_SIZE) < 0) {
		if (errno == EAGAIN || errno == EINTR)
			continue;

		throw std::runtime_error("Bluetooth socket failed.");
	}

	if (buf[0] == HCI_EVENT_PKT) {
		hci_event_hdr *hdr = (hci_event_hdr *) (buf + 1);

		if (hdr->evt == EVT_LE_META_EVENT) {
			evt_le_meta_event *evt = (evt_le_meta_event *) ((uint8_t *)hdr + HCI_EVENT_HDR_SIZE);

			if (evt->subevent == EVT_LE_ADVERTISING_REPORT) {
				uint8_t report_cnt = evt->data[0];
				
				if (report_cnt != 1) { // TODO: Implement > 1 reports per event
					std::cout << "Multiple advertising reports in one event is not yet supported.";
				        std::cout << " Only first report will be processed." << std::endl;
				}

				le_advertising_info *info = (le_advertising_info *)(evt->data + 1); // first report
				
				report.bdaddr_type = info->bdaddr_type;
				bacpy(&report.bdaddr, &info->bdaddr);
				report.rssi = *((int8_t *)info + LE_ADVERTISING_INFO_SIZE + info->length);
			}
		}
	}

	return report;
}

///////// TODO: Remove later
int main(int argc, char** argv)
{
	const bdaddr_t* bdaddr;
	BLEAdvScanner* scanner;
	char str[18];
       
	scanner = new BLEAdvScanner();
	
	std::cout << "Using HCI device " << scanner->getHCIDevice() << std::endl;
	
	bdaddr = scanner->getBDAddr();
	ba2str(bdaddr, str);
	std::cout << "Bluetooth address: " << str << std::endl;

	scanner->setPassiveMode();
	std::cout << "Set up passive scanning mode." << std::endl;

	scanner->start(true); // Inform about duplicite advertising info

	adv_report report;
	report = scanner->scan();
	ba2str(&report.bdaddr, str);
	std::cout << "Advertising found" << std::endl;
	std::cout << "   BDADDR: " << str;
       	switch (report.bdaddr_type) {
		case 0x00:
			std::cout << " (Public Device)";
			break;
		case 0x01:
			std::cout << " (Random Device)";
			break;
		case 0x02:
			std::cout << " (Public Identity)";
			break;
		case 0x03:
			std::cout << " (Random Identity)";
			break;
		default:
			std::cout << " (Invalid type)";
			break;
	}
	std::cout << std::endl;
	std::cout << "   RSSI: " << (int)report.rssi << std::endl;

	scanner->stop();
	
	delete scanner;

	return 1;
}
