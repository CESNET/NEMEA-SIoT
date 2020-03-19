#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <csignal>
#include <ctime>
#include <getopt.h>
#include <iostream>
#include <libtrap/trap.h>
#include <string>
#include <unirec/unirec.h>
#include <unistd.h>

#include "Configuration.h"
#include "fields.h"

#define DEFAULT_CONFIG "ble-conn-guard.ini"

UR_FIELDS (
	// Generic fields
	time    TIMESTAMP,
	macaddr INCIDENT_DEV_ADDR,  // Bluetooth address of the device
	uint32  ALERT_CODE,
	string  CAPTION,
	uint32  DURATION
	
  uint8   ATYPE, // Address type: 0 = public, 1 = random
)

trap_module_info_t *module_info = NULL;

#define MODULE_BASIC_INFO(BASIC) \
	BASIC("BLE Connection Guard", \
		"This module receives UniRec containing information about BLE connections" \
		"and sends alerts if the connection is not allowed according to setting.", \
    1, 1)

#define MODULE_PARAMS(PARAM) \
	PARAM('I', "ignore-in-eof", "Do not terminate on incomming termination message.", no_argument, "none") \
	PARAM('c', "config", "Use this configuration file. (Default is ./ble-conn-guard.ini)", required_argument, "string")

static bool BLEConnGuard_run = true;

TRAP_DEFAULT_SIGNAL_HANDLER(BLEConnGuard_run = false)

int main(int argc, char **argv)
{
  int retval = 0;
	signed char opt;
	bool ignore_eof = 0; // Ignore EOF input parameter flag
  char *confFile = NULL;
  Configuration* config = NULL;
	
  /* UniRec variables */
	ur_template_t *in_tmplt = NULL, *out_tmplt = NULL;
	void *out_rec = NULL;
	
  /* UniRec Initialization */
	INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)

	TRAP_DEFAULT_INITIALIZATION(argc, argv, *module_info);

	TRAP_REGISTER_DEFAULT_SIGNAL_HANDLER();
	
  /*
	 * Parse program arguments defined by MODULE_PARAMS macro with getopt() function (getopt_long() if available)
	 * This macro is defined in config.h file generated by configure script
	 */
	while ((opt = TRAP_GETOPT(argc, argv, module_getopt_string, long_options)) != -1) {
		switch (opt) {
			case 'I':
				ignore_eof = 1;
				break;
      case 'c':
        confFile = optarg;
        break;
			default:
				std::cerr << "Error: Invalid parameter." << std::endl;
				retval = 1;
				goto unirec_cleanup;
		}
	}
	
  in_tmplt = ur_create_input_template(0, "TIMESTAMP,INCIDENT_DEV_ADDR,ALERT_CODE,CAPTION,ATYPE,DURATION", NULL);
	if (in_tmplt == NULL) {
		std::cerr << "Error: Failed to create UniRec input template." << std::endl;
		retval = 2;
		goto unirec_cleanup;
	}

	out_tmplt = ur_create_output_template(0, "TIMESTAMP,INCIDENT_DEV_ADDR,ALERT_CODE,CAPTION,ATYPE,DURATION", NULL);
	if (out_tmplt == NULL) {
		std::cerr << "Error: Failed to create UniRec output template." << std::endl;
		retval = 2;
		goto unirec_cleanup;
	}
	
  out_rec = ur_create_record(out_tmplt, UR_MAX_SIZE);
	if (out_rec == NULL) {
		std::cerr << "Error: Failed to create UniRec record." << std::endl;
		retval = 2;
		goto unirec_cleanup;
	}
  
  try {
    if (confFile == NULL)
      config = new Configuration(DEFAULT_CONFIG);
    else
      config = new Configuration(confFile);
  } catch (IOError& e) {
    retval = 3;
    std::cerr << e.what() << std::endl;
    goto unirec_cleanup;
  } catch (ParseError& e) {
    retval = 3;
    std::cerr << e.what() << std::endl;
    goto unirec_cleanup;
  }

	/* Main loop */
	while (BLEConnGuard_run) {
		const void *in_rec;
		uint16_t    in_rec_size;

    ur_time_t  timestamp;
    std::time_t sec;
    std::tm *time;

    mac_addr_t bdaddr;
		char bdaddrStr[MAC_STR_LEN];

		retval = TRAP_RECEIVE(0, in_rec, in_rec_size, in_tmplt);

		TRAP_DEFAULT_RECV_ERROR_HANDLING(retval, continue, break);

		// Check size of received data
		if (in_rec_size < ur_rec_fixlen_size(in_tmplt)) {

			if (in_rec_size == 1) { // EOF
				char dummy[1] = {0};
				trap_send(0, dummy, 1);
				trap_send_flush(0);
				if (ignore_eof)
					continue;
				break;
			} else if (in_rec_size < 1) { // Read error
        retval = 4;
				std::cerr << "Error: Read error occured." << std::endl;
				break;
			} else {
        retval = 4;
				std::cerr << "Error: Data with wrong size received." << std::endl;
				break;
			}
		}

    timestamp = ur_get(in_tmplt, in_rec, F_TIMESTAMP);
    sec = ur_time_get_sec(timestamp);
    time = std::localtime(&sec);
    if (time == NULL) {
      std::cerr << "Error: Received data with invalid timestamp." << std::endl;
      continue;
    }

    bdaddr    = ur_get(in_tmplt, in_rec, F_INCIDENT_DEV_ADDR);
    mac_to_str(&bdaddr, bdaddrStr);

    if (!config->allowedConnection(bdaddrStr, time)) {
      ur_copy_fields(out_tmplt, out_rec, in_tmplt, in_rec);
        
      trap_send(0, out_rec, ur_rec_size(out_tmplt, out_rec));
      TRAP_DEFAULT_SEND_ERROR_HANDLING(retval, continue, break);
    }
	}

  delete config;

unirec_cleanup:
	TRAP_DEFAULT_FINALIZATION();

	ur_free_template(in_tmplt);
	ur_free_template(out_tmplt);

	ur_free_record(out_rec);

	ur_finalize();

	FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)

 
  return retval;
}
