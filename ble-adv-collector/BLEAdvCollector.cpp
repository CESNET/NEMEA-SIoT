#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <csignal>
#include <errno.h>
#include <getopt.h>
#include <libtrap/trap.h>
#include <unirec/unirec.h>
#include <unistd.h>

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

static int terminate = 0;

TRAP_DEFAULT_SIGNAL_HANDLER(terminate = 1)


int main(int argc, char **argv)
{
	int retval = 0;
	
	signed char opt;
	uint16_t hci_dev = 0;
	
	ur_template_t *out_template = NULL;
	void *record = NULL;

	
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

	ur_free_template(out_template);

	TRAP_DEFAULT_FINALIZATION();
	
	FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)

	return retval;
}

/* vim: set ts=3 sw=3 et */
