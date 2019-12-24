/**
 * \file lora-distance.c
 * \brief LoRaWAN Detector of NEMEA module.
 * \author Erik Gresak <erik.gresak@vsb.cz>
 * \date 2018
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
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <signal.h>
#include <getopt.h>
#include <inttypes.h>
#include <libtrap/trap.h>
#include <unirec/unirec.h>
#include "fields.h"
#include "lora_packet.h"
#include <string.h>
#include <unistd.h>
#include "device_list.h"


/** Define structure for DeviceList */
struct dl_device {
    uint64_t DEV_ADDR;
    double BASE_RSSI;
    struct dl_device *next;
};

/** 
 * Statically defined fields contain time stamp record TIMESTAMP, device address  
 * DEV_ADDR, received signal strength Indicator RSSI, base received signal strength 
 * Indicator BASE_RSSI, variance for base (RSSI) VARIANCE and payload from message 
 * PHY_PAYLOAD. This values are captured from LoRaWAN packet.
 */
UR_FIELDS(
        // Input UniRec format
        // TODO: change data type to uint64
        string DEV_ADDR,
        string PHY_PAYLOAD,
        // Output UniRec format
        //TODO: change TIMESTAMP to time datatype
        uint64 TIMESTAMP,
        uint64 INCIDENT_DEV_ADDR,
        uint32 ALERT_CODE,
        string CAPTION,
        double RSSI,
        double BASE_RSSI,
        double VARIANCE
        // These values is possible to get from LoRa message
        //        string GW_ID,
        //        string NODE_MAC,
        //        uint32 US_COUNT,
        //        uint32 FRQ,
        //        uint32 RF_CHAIN,
        //        uint32 RX_CHAIN,
        //        string STATUS,
        //        uint32 SIZE,
        //        string MOD,
        //        uint32 BAD_WIDTH,
        //        uint32 SF,
        //        uint32 CODE_RATE,
        //        double SNR,
        //        string APP_EUI,
        //        string APP_NONCE,
        //        string DEV_EUI,
        //        string DEV_NONCE,
        //        string FCTRL,
        //        string FHDR,
        //        string F_OPTS,
        //        string F_PORT,
        //        string FRM_PAYLOAD,
        //        string LORA_PACKET,
        //        string MAC_PAYLOAD,
        //        string MHDR,
        //        string MIC,
        //        string NET_ID,
        //        uint64 AIR_TIME
        )

trap_module_info_t *module_info = NULL;


/**
 * Definition of basic module information - module name, module description, number of input and output interfaces
 */
#define MODULE_BASIC_INFO(BASIC) \
  BASIC("LoRaWAN Detection - Change distance", \
        "This detector serves for detection changing distance between device and gateway. Detection is for " \
        "fixed-position devices, if the attacker transfers the device, the RSSI (Received Signal Strength Indication) changes. " \
        "This may vary depending on the environment, such as weather. Therefore, it is possible to set the deviation for RSSI. " \
        "Base RSSI value is defined by the first received message from device to detector.", 1, 1)

/**
 * Definition of module parameters - every parameter has short_opt, long_opt, description,
 * flag whether an argument is required or it is optional and argument type which is NULL
 * in case the parameter does not need argument.
 * Module parameter argument types: int8, int16, int32, int64, uint8, uint16, uint32, uint64, float, string
 */
#define MODULE_PARAMS(PARAM) \
    PARAM('a', "variance", "Defines explicit variance, default value 10% (0.1).", required_argument, "double") \
    PARAM('I', "ignore-in-eof", "Do not terminate on incomming termination message.", no_argument, "none") \

/**
 * To define positional parameter ("param" instead of "-m param" or "--mult param"), use the following definition:
 * PARAM('-', "", "Parameter description", required_argument, "string")
 * There can by any argument type mentioned few lines before.
 * This parameter will be listed in Additional parameters in module help output
 */

static int stop = 0;

/**
 * Function to handle SIGTERM and SIGINT signals (used to stop the module)
 */
TRAP_DEFAULT_SIGNAL_HANDLER(stop = 1)

/**
 * Function trap finalization and print error.
 */
void trap_fin(char *arg) {
    fprintf(stderr, "%s\n", arg);
    TRAP_DEFAULT_FINALIZATION();
}

/** ---- MAIN ----- */
int main(int argc, char **argv) {
    int ret;
    signed char opt;
    int ignore_eof = 0; // Ignore EOF input parameter flag

    /** 
     * Default fields for calculate variance
     */
    double va = 0.1;

    /* **** TRAP initialization **** */

    /*
     * Macro allocates and initializes module_info structure according to MODULE_BASIC_INFO and MODULE_PARAMS
     * definitions on the lines 118 and 131 of this file. It also creates a string with short_opt letters for getopt
     * function called "module_getopt_string" and long_options field for getopt_long function in variable "long_options"
     */
    INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)

    /*
     * Let TRAP library parse program arguments, extract its parameters and initialize module interfaces
     */
    TRAP_DEFAULT_INITIALIZATION(argc, argv, *module_info);

    /*
     * Register signal handler.
     */
    TRAP_REGISTER_DEFAULT_SIGNAL_HANDLER();

    /*
     * Parse program arguments defined by MODULE_PARAMS macro with getopt() function (getopt_long() if available)
     * This macro is defined in config.h file generated by configure script
     */
    while ((opt = TRAP_GETOPT(argc, argv, module_getopt_string, long_options)) != -1) {
        switch (opt) {
            case 'a':
                sscanf(optarg, "%lf", &va);
                if ((va >= 0) && (va <= 1))
                    break;
                trap_fin("Invalid arguments variance 0.0 - 1.0\n");
                FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS);
                return -1;
            case 'I':
                ignore_eof = 1;
                break;
            default:
                trap_fin("Invalid arguments.\n");
                FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS);
                return -1;
        }
    }

    /** Create Input UniRec templates */
    ur_template_t *in_tmplt = ur_create_input_template(0, "TIMESTAMP,RSSI,PHY_PAYLOAD", NULL);
    if (in_tmplt == NULL) {
        ur_free_template(in_tmplt);
        fprintf(stderr, "Error: Input template could not be created.\n");
        return -1;
    }

    /** Create Output UniRec templates */
    ur_template_t *out_tmplt = ur_create_output_template(0, "INCIDENT_DEV_ADDR,TIMESTAMP,BASE_RSSI,RSSI,VARIANCE,CAPTION,ALERT_CODE", NULL);
    if (out_tmplt == NULL) {
        ur_free_template(in_tmplt);
        ur_free_template(out_tmplt);
        fprintf(stderr, "Error: Output template could not be created.\n");
        return -1;
    }

    /** Allocate memory for output record */
    void *out_rec = ur_create_record(out_tmplt, 530);
    if (out_rec == NULL) {
        ur_free_template(in_tmplt);
        ur_free_template(out_tmplt);
        ur_free_record(out_rec);
        fprintf(stderr, "Error: Memory allocation problem (output record).\n");
        return -1;
    }

    /**  
     * Main processing loop
     * Read data from input, process them and write to output  
     */
    while (!stop) {
        const void *in_rec;
        uint16_t in_rec_size;

        /** 
         * Receive data from input interface 0.
         * Block if data are not available immediately (unless a timeout is set using trap_ifcctl)
         */
        ret = TRAP_RECEIVE(0, in_rec, in_rec_size, in_tmplt);

        /** Handle possible errors */
        TRAP_DEFAULT_RECV_ERROR_HANDLING(ret, continue, break);

        /** Indicates EOF */
        if (in_rec_size == 1) {
            char dummy[1] = {0};
            trap_send(0, dummy, 1);
            trap_send_flush(0);
            if (!ignore_eof)
                break;
        }

        /** Check size payload min/max */
        uint32_t size = ur_get_len(in_tmplt, in_rec, F_PHY_PAYLOAD);
        if(size < 14 || size > 512)
            continue;
        
        /** Initialization physical payload for parsing and reversing octet fields. */
        lr_initialization(ur_get_ptr(in_tmplt, in_rec, F_PHY_PAYLOAD));

        if (DevAddr == NULL)
            continue;
        
        /** Identity message type */
        if (lr_is_join_accept_message()) {
            ur_set_string(out_tmplt, out_rec, F_DEV_ADDR, DevAddr);
        } else if (lr_is_data_message()) {
            ur_set_string(out_tmplt, out_rec, F_DEV_ADDR, DevAddr);
        }

        /** 
         * DeviceList
         * Information is retrieved from incoming physical payload (PHYPayload) by parsing 
         * and revers octets. Each row in DeviceList contains device a BASE_RSSI of 
         * received message. The device address (DevAddr) is used as the index.
         */

        /** 
         * Load last data from Device
         */
        uint64_t dev_addr = lr_uint8_to_uint64(lr_arr_to_uint8(DevAddr));
        
        struct dl_device *pre = dl_get_device(dev_addr);

        if (pre != NULL) {
            /**
             * Detection change distance
             * The example shows the attacker's identification where the detector is set 
             * to 10% variance. This means that for -119 dBm is variance -11.9 dBm. 
             * The minimum value is -130.9 dBm and maximum -107.1 dBm. An attacker is 
             * therefore detected because it does not fall within the range.
             */

            double variance = pre->BASE_RSSI * va;
            

            if (!(((pre->BASE_RSSI + variance) <= ur_get(in_tmplt, in_rec, F_RSSI)) && (ur_get(in_tmplt, in_rec, F_RSSI) <= (pre->BASE_RSSI - variance)))) {

                // calculate alert value difference
                double alert_value = 0;
                if ( (pre->BASE_RSSI + variance) > ur_get(in_tmplt, in_rec, F_RSSI) ){
                    alert_value = (pre->BASE_RSSI + variance) - ur_get(in_tmplt, in_rec, F_RSSI);
                } else {
                    alert_value = ur_get(in_tmplt, in_rec, F_RSSI) - (pre->BASE_RSSI - variance);
                }
                
                ur_set(out_tmplt, out_rec, F_TIMESTAMP, ur_get(in_tmplt, in_rec, F_TIMESTAMP));
                ur_set(out_tmplt, out_rec, F_INCIDENT_DEV_ADDR, dev_addr);
                ur_set(out_tmplt, out_rec, F_ALERT_CODE, 0);
                // Create Caption message from alert values
                char alert_str[100]; 
                sprintf(alert_str, "The device %ld exceeded allowed distance from gateway by %f dBm", dev_addr,-1*alert_value);
                ur_set_string(out_tmplt, out_rec, F_CAPTION, alert_str);

                ur_set(out_tmplt, out_rec, F_RSSI, ur_get(in_tmplt, in_rec, F_RSSI));
                ur_set(out_tmplt, out_rec, F_BASE_RSSI, pre->BASE_RSSI);
                ur_set(out_tmplt, out_rec, F_VARIANCE, va);
                ret = trap_send(0, out_rec, ur_rec_size(out_tmplt, out_rec));

                TRAP_DEFAULT_SEND_ERROR_HANDLING(ret, continue, break);
            }

        } else {
            /** 
             * Insert new device to DeviceList
             */
            dl_insert_device(dev_addr, ur_get(in_tmplt, in_rec, F_RSSI));
        }

        /** 
         * Free lora_packet and output record
         */
        lr_free();
    }


    /* **** Cleanup **** */

    /** 
     * Do all necessary cleanup in libtrap before exiting
     */
    TRAP_DEFAULT_FINALIZATION();

    /** 
     * Release allocated memory for module_info structure
     */
    FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)

    /** 
     *  Free unirec templates and output record
     */
    ur_free_record(out_rec);
    ur_free_template(in_tmplt);
    ur_free_template(out_tmplt);
    ur_finalize();


    return 0;
}
