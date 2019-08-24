#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <bluetooth/bluetooth.h>
#include <csignal>
#include <errno.h>
#include <getopt.h>
#include <iostream>
#include <libtrap/trap.h>
#include <unirec/unirec.h>
#include <unistd.h>

#include "BLEAdvScanner.h"
#include "fields.h"

using namespace std;

UR_FIELDS (
   time    TIMESTAMP,
   macaddr DEV_ADDR, // Bluetooth address of the device

   uint8   ATYPE, // Address type: 0 = public, 1 = random
   uint8   RSSI   // Signal strength
)

trap_module_info_t *module_info = NULL;

#define MODULE_BASIC_INFO(BASIC) \
	BASIC("BLE Advertisement Collector", \
			"This module switches bluetooth controller into continuous passive scanning and reports all discovered advertising packets.", \
			0, 1)

#define MODULE_PARAMS(PARAM) \
	PARAM('d', "dev", "HCI device id of Bluetooth controller to use.", no_argument, "uint16")

#define BDADDR_STR_SIZE 18

static bool BLEAdvCollector_run = true;

TRAP_DEFAULT_SIGNAL_HANDLER(BLEAdvCollector_run = false)


int main(int argc, char **argv)
{
	int retval = 0;
	
	/* User interface variables */
	signed char opt;
	uint16_t hci_dev = 0;
	
	/* UniRec variables */
	ur_template_t *out_template = NULL;
	void *record = NULL;

	/* Bluetooth variables */
	const bdaddr_t* bdaddr;
	adv_report report;
	BLEAdvScanner* scanner;
	char buf[BDADDR_STR_SIZE];
	
	/* UniRec Initialization */
	INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
	
	TRAP_DEFAULT_INITIALIZATION(argc, argv, *module_info);

	TRAP_REGISTER_DEFAULT_SIGNAL_HANDLER();

	while ((opt = TRAP_GETOPT(argc, argv, module_getopt_string, long_options)) != -1) {
		switch (opt) {
			case 'd':
				hci_dev = atoi(optarg);
				break;

			default:
				break;
		}
	}

	out_template = ur_create_output_template(0, "TIMESTAMP,DEV_ADDR,ATYPE,RSSI", NULL);
	if (out_template == NULL) {
		// TODO: Cleanup
	}

	/* BLE Advertisement Scanner initialization */
	scanner = new BLEAdvScanner();
	
	std::cout << "Using HCI device " << scanner->getHCIDevice() << std::endl;

	bdaddr = scanner->getBDAddr();
	ba2str(bdaddr, buf);
	std::cout << "Bluetooth address: " << buf << std::endl;

	scanner->setPassiveMode();
	std::cout << "Set up passive scanning mode." << std::endl;
	
	/* Main loop */
	// Start without filtering duplicities
	for (scanner->start(false); BLEAdvCollector_run; report = scanner->getAdvReport()) {
		ba2str(&report.bdaddr, buf);
		std::cout << "Advertising found" << std::endl;
		std::cout << "   BDADDR: " << buf;
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
	}
	scanner->stop();

	delete scanner;

	/* UniRec Cleanup */
	ur_free_template(out_template);

	TRAP_DEFAULT_FINALIZATION();
	
	FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)

	return retval;
}

/* vim: set ts=3 sw=3 et */
