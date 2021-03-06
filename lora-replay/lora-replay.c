/**
 * \file lora-replay.c
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

/** Define structure for BlackList */
struct dl_device {
    uint64_t DEV_ADDR;
    uint16_t LAST_FCNT;
    uint8_t RESTART;
    struct dl_device *next;
};

/** 
 * Statically defined fields contain time stamp record TIMESTAMP, device address 
 * DEV_ADDR, message counter FCNT and payload from message PHY_PAYLOAD. This values 
 * are captured from LoRaWAN packet.
 */
UR_FIELDS(
        time TIMESTAMP,
        uint64 DEV_ADDR,
        uint64 INCIDENT_DEV_ADDR,
        uint32 ALERT_CODE,
        uint16 FCNT,
        uint8 STATUS,
        uint8 MS_TYPE,
        string CAPTION
        )

trap_module_info_t *module_info = NULL;


/**
 * Definition of basic module information - module name, module description, number of input and output interfaces
 */
#define MODULE_BASIC_INFO(BASIC) \
  BASIC("LoRaWAN Detection - Replay attack ABP", \
        "This detector serves for detection replay attack in LoRaWAN infrastructure of ABP authentication method." \
        "The attacker is detected based on its behavior. The attack process begins with data storage of each device " \
        "captured within range. If an attacker captures a restart message for the device (Fcnt = 0), sends the last stored " \
        "message to the gateway from captured data where is device with the highest counter value. The attack " \
        "replicates the last message and sends it over the gateway to the server.Server assumes that it receives a " \
        "higher message than the attacker has sent and awaits this message. Sensor reboot and sending messages until " \
        "it reaches the counter of the attacking message. Sensor is dosing for this time. This attack can be harmful " \
        "for ABP activated end devices.\n" \
        "Attack detection is performed by saving data to the list (DeviceList). Information is retrieved from incoming " \
        "physical payload (PHYPayload) by parsing and revers octets. Each row in DeviceList contains device has a counter (FCnt) " \
        "of received message and information about restart the device (RESTART). If the device is restarted, RESTART " \
        "value is set to 1. The device address (DevAddr) is used as the row index. An attacker is recognized if his last message " \
        "is the same as the message after restarting the device. Identification of the attacker is based on same couture (FCnt).", 1, 1)

/**
 * Definition of module parameters - every parameter has short_opt, long_opt, description,
 * flag whether an argument is required or it is optional and argument type which is NULL
 * in case the parameter does not need argument.
 * Module parameter argument types: int8, int16, int32, int64, uint8, uint16, uint32, uint64, float, string
 */
#define MODULE_PARAMS(PARAM)  \
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


/** ---- MAIN ----- */
int main(int argc, char **argv) {
    int ret;
    signed char opt;
    int ignore_eof = 0; // Ignore EOF input parameter flag

    /* **** TRAP initialization **** */

    /*
     * Macro allocates and initializes module_info structure according to MODULE_BASIC_INFO and MODULE_PARAMS
     * definitions on the lines 88 and 110 of this file. It also creates a string with short_opt letters for getopt
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
            case 'I':
                ignore_eof = 1;
                break;
            default:
                fprintf(stderr, "Invalid arguments.\n");
                FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS);
                TRAP_DEFAULT_FINALIZATION();
                return -1;
        }
    }

    /** Create Input UniRec templates */
    ur_template_t *in_tmplt = ur_create_input_template(0, "TIMESTAMP,DEV_ADDR,FCNT,STATUS,MS_TYPE", NULL);
    if (in_tmplt == NULL) {
        ur_free_template(in_tmplt);
        fprintf(stderr, "Error: Input template could not be created.\n");
        return -1;
    }

    /** Create Output UniRec templates */
    ur_template_t *out_tmplt = ur_create_output_template(0, "INCIDENT_DEV_ADDR,TIMESTAMP,FCNT,ALERT_CODE,CAPTION", NULL);
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
        
        /** Check status message */
        if (ur_get(in_tmplt, in_rec, F_STATUS) != 16)
            continue;

        /** 
         * DeviceList
         * Information is retrieved from incoming physical payload (PHYPayload) 
         * by parsing and revers octets. Each row in DeviceList contains device 
         * has a counter (FCnt) of received message and information about 
         * restart the device (RESTART). If the device is restarted, RESTART 
         * value is set to 1. The device address (DevAddr) is used as the row 
         * index.
         */
        
        uint64_t dev_addr = ur_get(in_tmplt, in_rec, F_DEV_ADDR);
        uint16_t counter = ur_get(in_tmplt, in_rec, F_FCNT);
        uint8_t ms_type = ur_get(in_tmplt, in_rec, F_MS_TYPE);

        if (dev_addr == 0)
            continue;
        
        /** Identity message type */
        if ((ms_type >= 2) && (ms_type <= 5)) {
            ur_set(out_tmplt, out_rec, F_FCNT, counter);

            /** 
             * Load last data from Device
             */
            
            struct dl_device *pre = dl_get_device(dev_addr);

            if (pre != NULL) {
                /**
                 * Detection attack ABP
                 * Attack detection is performed by saving data to the list (DeviceList). 
                 * Information is retrieved from incoming physical payload (PHYPayload) 
                 * by parsing and revers octets. Each row in DeviceList contains device 
                 * has a counter (FCnt) of received message and information about restart 
                 * the device (RESTART). If the device is restarted, RESTART value is 
                 * set to 1. The device address (DevAddr) is used as the row index. An 
                 * attacker is recognized if his last message is the same as the 
                 * message after restarting the device. Identification of the attacker 
                 * is based on same couture (FCnt).
                 * 
                 */
                if ((pre->RESTART == 1) && (pre->LAST_FCNT == counter) && (counter != 0)) {
                    // Create alert message
                    ur_set(out_tmplt, out_rec, F_TIMESTAMP, ur_get(in_tmplt, in_rec, F_TIMESTAMP));
                    ur_set(out_tmplt, out_rec, F_ALERT_CODE, 0);
                    uint64_t dev_addr_id = ur_get(in_tmplt, in_rec, F_DEV_ADDR);
                    ur_set(out_tmplt, out_rec, F_INCIDENT_DEV_ADDR, dev_addr_id);
                    // Create caption message 
                    char alert_str[100];
                    sprintf(alert_str, "Replay attack has been detected for the device %ld",dev_addr_id);
                    ur_set_string(out_tmplt, out_rec, F_CAPTION, alert_str);
                    
                    ret = trap_send(0, out_rec, ur_rec_size(out_tmplt, out_rec));
                    TRAP_DEFAULT_SEND_ERROR_HANDLING(ret, continue, break);

                    pre->RESTART = 0;
                } else {
                    pre->RESTART = 0;
                }

                if (counter > 0) {
                    pre->LAST_FCNT = counter;
                }

                if (counter == 0) {
                    pre->RESTART = 1;
                }

            } else {
                /** 
                 * Insert new device to DeviceList
                 */
                dl_insert_device(dev_addr, counter);
            }

        }
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
