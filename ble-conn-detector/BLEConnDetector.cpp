#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <csignal>
#include <getopt.h>
#include <iostream>
#include <libtrap/trap.h>
#include <sstream>
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
   string  CAPTION,
   uint32  DURATION
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

	out_tmplt = ur_create_output_template(0, "TIMESTAMP,INCIDENT_DEV_ADDR,ALERT_CODE,CAPTION,ATYPE,DURATION", NULL);
	if (out_tmplt == NULL) {
		std::cerr << "Error: Failed to create UniRec output template." << std::endl;
		retval = 1;
		goto unirec_cleanup;
	}
	
  out_rec = ur_create_record(out_tmplt, UR_MAX_SIZE);
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

    try {
      detector->processAdvReport(&report);
    } catch (ConnectionFound evt) {

      mac_to_str(&evt.bdaddr, buf);
      
      std::stringstream caption_ss;
      caption_ss << "The device " << buf << " was in use for " << evt.usage_duration << "us.";
      
      std::string caption = caption_ss.str();

      std::cout << caption << std::endl;

      ur_set(out_tmplt, out_rec, F_TIMESTAMP, evt.timestamp);
      ur_set(out_tmplt, out_rec, F_INCIDENT_DEV_ADDR, evt.bdaddr);
      ur_set(out_tmplt, out_rec, F_ALERT_CODE, 0x01); // In future if other alerts exist, change to enum
      ur_set_string(out_tmplt, out_rec, F_CAPTION, caption.c_str());
      ur_set(out_tmplt, out_rec, F_ATYPE, evt.bdaddr_type);
      ur_set(out_tmplt, out_rec, F_DURATION, evt.usage_duration);

			trap_send(0, out_rec, ur_rec_size(out_tmplt, out_rec));
      TRAP_DEFAULT_SEND_ERROR_HANDLING(retval, continue, break);
/*
      std::cout << "ConnectionFound(" << evt.timestamp;
      std::cout << ", " << buf;
      switch (evt.bdaddr_type) {
        case 0x00:
          std::cout << " (Public)";
          break;
        case 0x01:
          std::cout << " (Random)";
          break;
        default:
          std::cout << " (Unknown)";
          break;
      }
      std::cout << ", " << evt.usage_duration << ")" << std::endl;
*/
    }
  }

  delete detector;

unirec_cleanup:
	/* UniRec Cleanup */
	TRAP_DEFAULT_FINALIZATION();
	
  ur_free_template(in_tmplt);
	ur_free_template(out_tmplt);
	
	ur_free_record(out_rec);

	ur_finalize();
	
	FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)


	return retval;
}

/* vim: set ts=3 sw=3 et */
