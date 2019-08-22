
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
	le_set_scan_parameters_cp params;
	uint8_t buf[HCI_MAX_EVENT_SIZE];

	memset(&params, 0, LE_SET_SCAN_PARAMETERS_CP_SIZE);
	params.type            = 0x00; // Passive scanning
	params.interval        = htobs(0x0010); // Minimal interval between scans => 10ms
	params.window          = htobs(0x0010); // The duration of actual scanning in one interval
	params.own_bdaddr_type = 0x00; // Public address
	params.filter          = 0x00; // Accept all ADV packets except directed ADV for different devices
	                      // 0x02 Accept all ADV packets except where initiator's identity address
	                      //      does not address this device

	if ( hci_send_cmd(sock,
		OGF_LE_CTL, OCF_LE_SET_SCAN_PARAMETERS,
		LE_SET_SCAN_PARAMETERS_CP_SIZE, &params) < 0) {
		
		throw std::runtime_error("Failed to set up passive mode.");
	}

	uint8_t attempt = 10;
	bool done = false;
	while (!done && attempt--) {
		while (read(sock, buf, HCI_MAX_EVENT_SIZE) < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			
			throw std::runtime_error("Failed to read from socket.");
		}

		if (buf[0] == HCI_EVENT_PKT) {
			hci_event_hdr *hdr = (hci_event_hdr *) (buf + 1);
			evt_cmd_complete *cc;
			uint8_t status;

			switch (hdr->evt) {
				case EVT_CMD_COMPLETE:
					cc = (evt_cmd_complete *) (buf + 1 + HCI_EVENT_HDR_SIZE);
					if (cc->opcode != htobs(cmd_opcode_pack(OGF_LE_CTL, OCF_LE_SET_SCAN_PARAMETERS)))
						continue; // Does not belong to the command issued
					
					status = *((uint8_t*)cc + EVT_CMD_COMPLETE_SIZE);
					if (status != 0x00) // 0x00 = Command succeeded
						throw std::runtime_error("LE_Set_Scan_Parameters command failed.");
					
					done = true;
					break;
				default:
					continue;
			}
		} // if (buf[0] == HCI_EVENT_PKT)

	} // while (try--)
}

void BLEAdvScanner::start(void)
{
	struct hci_request req;
	le_set_scan_enable_cp params;
	uint8_t status; // Response is a single status byte
	int err;

	params.enable = true;
	params.filter_dup = false; // Inform about duplicite advertising info
	
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
	
}

void BLEAdvScanner::stop(void)
{
	struct hci_request req;
	le_set_scan_enable_cp params;
	uint8_t status; // Response is a single status byte
	int err;

	params.enable = false;
	params.filter_dup = true; // not needed, but nice to set the default
	
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

	scanner->start();

	scanner->stop();
	
	delete scanner;

	return 1;
}
