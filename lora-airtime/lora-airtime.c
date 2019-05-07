/**
 * \file lora-airtime.c
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
#include "black_list.h"

/** Define structure for BlackList */
struct bl_device {
    char DEV_ADDR[8];
    double AIR_TIME;
    uint8_t ENABLE;
    uint64_t TIMESTAMP;
    uint16_t LAST_FCNT;
    uint8_t RESTART;
    struct bl_device *next;
};

/** 
 * Statically defined fields contain size payload SIZE, spreading factor SF, 
 * band width BAD_WIDTH, code rate CODE_RATE, time stamp record TIMESTAMP, 
 * device address DEV_ADDR, air-time AIR_TIME, enable device ENABLE, network 
 * session key NWK_SKEY, application session key APP_SKEY and payload from 
 * message PHY_PAYLOAD. This values are captured from LoRaWAN packet.
 */
UR_FIELDS(
        uint64 TIMESTAMP,
        uint32 SIZE,
        uint32 BAD_WIDTH,
        uint32 SF,
        uint32 CODE_RATE,
        string DEV_ADDR,
        string PHY_PAYLOAD,
        uint64 AIR_TIME,
        string NWK_SKEY,
        string APP_SKEY
//        uint8 ENABLE,
//        string GW_ID,
//        string NODE_MAC,
//        uint32 US_COUNT,
//        uint32 FRQ,
//        uint32 RF_CHAIN,
//        uint32 RX_CHAIN,
//        string STATUS,
//        string MOD,
//        double RSSI,
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
//        string NET_ID
        )

trap_module_info_t *module_info = NULL;


/**
 * Definition of basic module information - module name, module description, number of input and output interfaces
 */
#define MODULE_BASIC_INFO(BASIC) \
  BASIC("LoRaWAN Detection - Airtime regulations", \
        "This detector serves for LoRaWAN monitoring air time of individual sensors. " \
        "The detector can decode payload based on Network Sesion Key and Application Sesion Key. " \
        "The input of this detector is a fields contain size payload SIZE, spreding " \
        "factor SF, band width BAD_WIDTH, code rate CODE_RATE, time stamp record TIMESTAMP " \
        "and payload from message PHY_PAYLOAD. This values are captured from LoRaWAN packet.", 1, 1)

/**
 * Definition of module parameters - every parameter has short_opt, long_opt, description,
 * flag whether an argument is required or it is optional and argument type which is NULL
 * in case the parameter does not need argument.
 * Module parameter argument types: int8, int16, int32, int64, uint8, uint16, uint32, uint64, float, string
 */
#define MODULE_PARAMS(PARAM) \
  PARAM('e', "header", "Defines explicit header 1/0 (true/false), default value 1 (true).", required_argument, "int") \
  PARAM('r', "data-rate", "Low data rate optimization 1/0 (true/false)", required_argument, "int") \
  PARAM('p', "preamble", "Preamble symbol is defined for all regions in LoRaWAN 1.0 standard is 8, this is a default value.", required_argument, "int") \
  PARAM('d', "dutycycle", "Defines time between packet subsequence starts, default value dutycycle is 0.10. Dutycycle is expressed as a percentage.", required_argument, "double")

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
    fprintf(stderr, "%s\n" ,arg);
    TRAP_DEFAULT_FINALIZATION();
}

/** ---- MAIN ----- */
int main(int argc, char **argv) {
    int ret;
    signed char opt;

    /** 
     * Default fields for calculate air-time
     */
    int hd = 1;
    int dr = 0;
    int ps = 8;
    double dt = 0.1;

    /* **** TRAP initialization **** */

    /*
     * Macro allocates and initializes module_info structure according to MODULE_BASIC_INFO and MODULE_PARAMS
     * definitions on the lines 100 and 114 of this file. It also creates a string with short_opt letters for getopt
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
            case 'e':
                sscanf(optarg, "%d", &hd);
                if ((hd == 0) || (hd == 1))
                    break;
                trap_fin("Invalid arguments defines explicit header 1/0 (true/false) -e.\n");
                FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS);
                return -1;
            case 'r':
                sscanf(optarg, "%d", &dr);
                if ((dr == 0) || (dr == 1))
                    break;
                trap_fin("Invalid arguments low data rate 1/0 (true/false) -r.\n");
                FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS);
                return -1;
            case 'p':
                sscanf(optarg, "%d", &ps);
                if ((ps >= 0) || (ps <= 100))
                    break;
                trap_fin("Invalid arguments preamble symbol 0 - 100 -p.\n");
                FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS);
                return -1;
            case 'd':
                sscanf(optarg, "%lf", &dt);
                if ((dt >= 0) || (dt <= 100))
                    break;
                trap_fin("Invalid arguments dutycycle 0 - 100% -d.\n");
                FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS);
                return -1;
            default:
                trap_fin("Invalid arguments.\n");
                FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS);
                return -1;
        }
    }

    /** Create Input UniRec templates */
    ur_template_t *in_tmplt = ur_create_input_template(0, "SIZE,SF,BAD_WIDTH,CODE_RATE,TIMESTAMP,PHY_PAYLOAD", NULL);
    if (in_tmplt == NULL) {
        ur_free_template(in_tmplt);
        fprintf(stderr, "Error: Input template could not be created.\n");
        return -1;
    }

    /** Create Output UniRec templates */
    ur_template_t *out_tmplt = ur_create_output_template(0, "DEV_ADDR,TIMESTAMP,AIR_TIME,PHY_PAYLOAD", NULL);
    if (out_tmplt == NULL) {
        ur_free_template(in_tmplt);
        ur_free_template(out_tmplt);
        fprintf(stderr, "Error: Output template could not be created.\n");
        return -1;
    }

    /** Allocate memory for output record */
    void *out_rec = ur_create_record(out_tmplt, 512);
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

        /** Check size payload min/max */
        uint32_t size = ur_get(in_tmplt, in_rec, F_SIZE);
        if(size < 14 || size > 512)
            continue;
        
        /** Initialization physical payload for parsing and reversing octet fields. */
        lr_initialization(ur_get_ptr(in_tmplt, in_rec, F_PHY_PAYLOAD));

        ur_set_string(out_tmplt, out_rec, F_PHY_PAYLOAD, PHYPayload);

        /** Example code for decode LoRaWAN packet.
         *  if ((ur_get_len(in_tmplt, in_rec, F_NWK_SKEY) == 32) && (ur_get_len(in_tmplt, in_rec, F_APP_SKEY) == 32)) {
         *      ur_set_string(out_tmplt, out_rec, F_PHY_PAYLOAD, lr_uint8_to_string(lr_decode(
         *      lr_arr_to_uint8(ur_get_var_as_str(in_tmplt, in_rec, F_NWK_SKEY)),
         *      lr_arr_to_uint8(ur_get_var_as_str(in_tmplt, in_rec, F_APP_SKEY)))));
         *  }
         */

        /** Identity message type */
        if (lr_is_join_accept_message()) {
            ur_set_string(out_tmplt, out_rec, F_DEV_ADDR, DevAddr);
        } else if (lr_is_data_message()) {
            ur_set_string(out_tmplt, out_rec, F_DEV_ADDR, DevAddr);
        }

        /** 
         * Method for calculate time between packet subsequent starts: 
         * [uint32_t] pay_size - Total payload size, 
         * [uint8_t]  header - Explicit header 1 / 0, true / false
         * [uint8_t]  dr - Low data rate optimization 1 / 0, enable / disable
         * [uint32_t] sf - Spreading factor SF7 - SF12
         * [uint32_t] cd_rate - Error correction coding 4/5 - 4/8 
         * [uint32_t] prem_sym - Preamble symbol is defined for all regions 
         *            in LoRaWAN 1.0 standard is 8, this is a default value.
         * [uint32_t] band - Typically Bandwidth is 125 KHz
         * [double]   duty_cycle - Duty Cycle for EU regulation is 0.10%
         */
        double airtime = lr_airtime_calculate(ur_get(in_tmplt, in_rec, F_SIZE), hd, dr, ur_get(in_tmplt, in_rec, F_SF)
                , ur_get(in_tmplt, in_rec, F_CODE_RATE), ps, ur_get(in_tmplt, in_rec, F_BAD_WIDTH), dt);
        uint64_t timestamp = ur_get(in_tmplt, in_rec, F_TIMESTAMP);

        /** 
         * BlackList
         * Contain sensors list with identification field DevAddr. Blocking 
         * list that allow block messages from LoRaWAN infrastructure that 
         * have a information of history and actual air-time.
         */
        struct bl_device *pre = bl_get_device(DevAddr);

        if (pre != NULL) {
            /** 
             * Edit fields from exists device 
             */
            if ((pre->TIMESTAMP + pre->AIR_TIME) <= timestamp)
                pre->ENABLE = 0;
            else {
                pre->ENABLE = 1;

                pre->AIR_TIME = airtime;
                pre->TIMESTAMP = timestamp;

                if ((module_info->num_ifc_out == 1) && (timestamp != 0)) {
                    ur_set(out_tmplt, out_rec, F_AIR_TIME, pre->AIR_TIME);
                    ur_set(out_tmplt, out_rec, F_TIMESTAMP, pre->TIMESTAMP);

                    ret = trap_send(0, out_rec, ur_rec_size(out_tmplt, out_rec));
                    TRAP_DEFAULT_SEND_ERROR_HANDLING(ret, continue, break);
                }
            }
        } else {
            /** 
             * Insert new device to BlackList
             */
            bl_insert_device(DevAddr, timestamp, airtime, 1);
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
