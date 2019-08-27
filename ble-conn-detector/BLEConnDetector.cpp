#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <csignal>
#include <getopt.h>
#include <iostream>
#include <libtrap/trap.h>
#include <unirec/unirec.h>
#include <unistd.h>

#include "BLEConnDetector.h"
#include "fields.h"

UR_FIELDS (
   // Generic fields
   time    TIMESTAMP,
   
   uint8   ATYPE, // Address type: 0 = public, 1 = random
   
   // Input fields
   macaddr DEV_ADDR, // Bluetooth address of the device
   
   int8    RSSI,  // Signal strength

   // Output fields
   macaddr INCIDENT_DEV_ADDR,
   uint32  ALERT_CODE,
   string  CAPTION
)

trap_module_info_t *module_info = NULL;

#define MODULE_BASIC_INFO(BASIC) \
	BASIC("BLE Connection Detector", \
			"This module receives UniRec containing information about advertising packets " \
			"and detects when a device is in use.", 1, 1)

#define MODULE_PARAMS(PARAM)

static bool BLEConnDetector_run = true;

TRAP_DEFAULT_SIGNAL_HANDLER(BLEConnDetector_run = false)
    
int main(int argc, char **argv)
{
	int retval = 0;
	
	/* UniRec variables */
	ur_template_t *in_tmplt = NULL, *out_tmplt = NULL;
	void *out_rec = NULL;

  /* Detector variables */
  BLEConnDetector* detector;
	
  /* UniRec Initialization */
	INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
	
	TRAP_DEFAULT_INITIALIZATION(argc, argv, *module_info);

	TRAP_REGISTER_DEFAULT_SIGNAL_HANDLER();
	
	in_tmplt = ur_create_input_template(0, "TIMESTAMP,DEV_ADDR,ATYPE,RSSI", NULL);
	if (in_tmplt == NULL) {
		std::cerr << "Error: Failed to create UniRec input template." << std::endl;
		retval = 1;
		goto unirec_cleanup;
	}

	out_tmplt = ur_create_output_template(0, "TIMESTAMP,INCIDENT_DEV_ADDR,ALERT_CODE,CAPTION,ATYPE", NULL);
	if (out_tmplt == NULL) {
		std::cerr << "Error: Failed to create UniRec output template." << std::endl;
		retval = 1;
		goto unirec_cleanup;
	}
	
  out_rec = ur_create_record(out_tmplt, 0);
	if (out_rec == NULL) {
		std::cerr << "Error: Failed to create UniRec record." << std::endl;
		retval = 1;
		goto unirec_cleanup;
	}

  /* Detector Initialization */
  detector = new BLEConnDetector();

	/* Main loop */
  while (BLEConnDetector_run) {
    const void *in_rec;
    uint16_t    in_rec_size;
    adv_report  report;

    char buf[MAC_STR_LEN];

    retval = TRAP_RECEIVE(0, in_rec, in_rec_size, in_tmplt);

    TRAP_DEFAULT_RECV_ERROR_HANDLING(retval, continue, break);

    // Check size of received data
    if (in_rec_size < ur_rec_fixlen_size(in_tmplt)) {
      if (in_rec_size <= 1) { // End of data
        break;
      } else {
        std::cerr << "Error: Data with wrong size received." << std::endl;
        break;
      }
    }

    report.timestamp   = ur_get(in_tmplt, in_rec, F_TIMESTAMP);
    report.bdaddr      = ur_get(in_tmplt, in_rec, F_DEV_ADDR);
    report.bdaddr_type = ur_get(in_tmplt, in_rec, F_ATYPE);
    report.rssi        = ur_get(in_tmplt, in_rec, F_RSSI);

    detector->processAdvReport(&report);
/*
    mac_to_str(&report.bdaddr, buf);

    std::cout << report.timestamp << " Received advertisement report" << std::endl;
    std::cout << "\tFrom: " << buf;
    switch (report.bdaddr_type) {
      case 0x00:
        std::cout << " (Public)" << std::endl;
        break;
      case 0x01:
        std::cout << " (Random)" << std::endl;
        break;
      default:
        std::cout << " (Unknown)" << std::endl;
        break;
    }
    std::cout << "\tRSSI: " << (int)report.rssi << std::endl;
*/
  }

  delete detector;

unirec_cleanup:
	/* UniRec Cleanup */
	ur_free_record(out_rec);
	
  ur_free_template(in_tmplt);
	ur_free_template(out_tmplt);

	TRAP_DEFAULT_FINALIZATION();
	
	FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)


	return retval;
}

/* vim: set ts=3 sw=3 et */
