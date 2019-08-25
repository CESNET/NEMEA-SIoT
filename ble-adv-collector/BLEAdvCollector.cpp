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
	PARAM('d', "dev", "HCI device id of Bluetooth controller to use.", required_argument, "uint16")

#define BDADDR_STR_SIZE 18

static bool BLEAdvCollector_run = true;

TRAP_DEFAULT_SIGNAL_HANDLER(BLEAdvCollector_run = false)


int main(int argc, char **argv)
{
	int retval = 0;
	
	/* User interface variables */
	signed char opt;
	bool custom_hci_flag = false;
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
				custom_hci_flag = true;
				hci_dev = atoi(optarg);
				break;

			default:
				std::cerr << "Error: Invalid arguments." << std::endl;
				FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
				return 1;
		}
	}

	out_template = ur_create_output_template(0, "TIMESTAMP,DEV_ADDR,ATYPE,RSSI", NULL);
	if (out_template == NULL) {
		std::cerr << "Error: Failed to create UniRec output template." << std::endl;
		retval = 1;
		goto unirec_cleanup;
	}

	record = ur_create_record(out_template, 0);
	if (record == NULL) {
		std::cerr << "Error: Failed to create UniRec record." << std::endl;
		retval = 1;
		goto unirec_cleanup;
	}

	/* BLE Advertisement Scanner initialization */
	try {
		if (custom_hci_flag) {
			scanner = new BLEAdvScanner(hci_dev);
		} else {
			scanner = new BLEAdvScanner();
		}
	} catch (std::runtime_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		retval = 1;
		goto unirec_cleanup;
	}
	
	std::cout << "Using HCI device " << scanner->getHCIDevice() << std::endl;

	bdaddr = scanner->getBDAddr();
	ba2str(bdaddr, buf);
	std::cout << "Bluetooth address: " << buf << std::endl;

	try {
		scanner->setPassiveMode();
		std::cout << "Set up passive scanning mode." << std::endl;
	
	/* Main loop */
		// Start without filtering duplicities
		for (scanner->start(false); BLEAdvCollector_run; report = scanner->getAdvReport()) {

			ur_time_t timestamp = ur_time_from_sec_msec(
				report.timestamp.tv_sec,
				report.timestamp.tv_usec / 1000);

			mac_addr_t bdaddr = mac_from_bytes(report.bdaddr.b);

			ur_set(out_template, record, F_TIMESTAMP, timestamp);
			ur_set(out_template, record, F_DEV_ADDR, bdaddr);
			ur_set(out_template, record, F_ATYPE, report.bdaddr_type);
			ur_set(out_template, record, F_RSSI, report.rssi);
		
			trap_send(0, record, ur_rec_size(out_template, record));
		}
		scanner->stop();
	} catch (std::runtime_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		retval = 2;
	}

	delete scanner;

unirec_cleanup:
	/* UniRec Cleanup */
	ur_free_record(record);
	ur_free_template(out_template);

	TRAP_DEFAULT_FINALIZATION();
	
	FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)

	return retval;
}

/* vim: set ts=3 sw=3 et */
