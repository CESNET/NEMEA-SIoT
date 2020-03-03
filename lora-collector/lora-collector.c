/**
 * \file lora-collector.c
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

/**
 * Implement Semtech
 * Description: Configure LoRa concentrator and record received packets in a log file
 * License: Revised BSD License, see LICENSE.TXT file include in the project
 * Maintainer: Sylvain Miermont
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <stddef.h> 
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <inttypes.h>
#include <libtrap/trap.h>
#include <unirec/unirec.h>
#include "fields.h"
#include "lora_packet.h"
#include <string.h>
#include "device_list.h"

#include "parson.h"
#include "libloragw/inc/loragw_hal.h"

/** Maximum message size */
#define MAX_MSG_SIZE 1000

/** Private Macros */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define MSG(args...) if(debug) fprintf(stderr, args)

/** Default fields for calculate variance */
static int debug = 0; /* 1 -> application starting debugging */

/** signal handling variables */
struct sigaction sigact; /* SIGQUIT&SIGINT&SIGTERM signal handling */
static int exit_sig = 0; /* 1 -> application terminates cleanly (shut down hardware, close open files, etc) */
static int quit_sig = 0; /* 1 -> application terminates without shutting down the hardware */

struct counterLOG {
    int cnt_pkt_log, cnt_bad_pkt_log, cnt_all_pkt_log;
};

/* configuration variables needed by the application  */
uint64_t lgwm = 0; /* LoRa gateway MAC address */
char lgwm_str[17];

/* clock and log file management */
time_t now_time;
time_t log_start_time;
char log_file_name[64];

/* Default variables for count logger */
struct counterLOG st_counter;
int cl = 0;

/** Private function declaration */
static void sig_handler(int sigio);
int parse_SX1301_configuration(const char * conf_file);
int parse_gateway_configuration(const char * conf_file);
void usage(void);

/* Logger function declaration */
void start_log(void);
void change_log(void);

/** Private function definition */
static void sig_handler(int sigio) {
    if (sigio == SIGQUIT) {
        quit_sig = 1;
        ;
    } else if ((sigio == SIGINT) || (sigio == SIGTERM)) {
        exit_sig = 1;
    }
}

int parse_SX1301_configuration(const char * conf_file) {
    int i;
    const char conf_obj[] = "SX1301_conf";
    char param_name[32]; /* used to generate variable parameter names */
    const char *str; /* used to store string value from JSON object */
    struct lgw_conf_board_s boardconf;
    struct lgw_conf_rxrf_s rfconf;
    struct lgw_conf_rxif_s ifconf;
    JSON_Value *root_val;
    JSON_Object *root = NULL;
    JSON_Object *conf = NULL;
    JSON_Value *val;
    uint32_t sf, bw;

    /* try to parse JSON */
    root_val = json_parse_file_with_comments(conf_file);
    root = json_value_get_object(root_val);
    if (root == NULL) {
        MSG("ERROR: %s id not a valid JSON file\n", conf_file);
        exit(EXIT_FAILURE);
    }
    conf = json_object_get_object(root, conf_obj);
    if (conf == NULL) {
        MSG("INFO: %s does not contain a JSON object named %s\n", conf_file, conf_obj);
        return -1;
    } else {
        MSG("INFO: %s does contain a JSON object named %s, parsing SX1301 parameters\n", conf_file, conf_obj);
    }

    /* set board configuration */
    memset(&boardconf, 0, sizeof boardconf); /* initialize configuration structure */
    val = json_object_get_value(conf, "lorawan_public"); /* fetch value (if possible) */
    if (json_value_get_type(val) == JSONBoolean) {
        boardconf.lorawan_public = (bool) json_value_get_boolean(val);
    } else {
        MSG("WARNING: Data type for lorawan_public seems wrong, please check\n");
        boardconf.lorawan_public = false;
    }
    val = json_object_get_value(conf, "clksrc"); /* fetch value (if possible) */
    if (json_value_get_type(val) == JSONNumber) {
        boardconf.clksrc = (uint8_t) json_value_get_number(val);
    } else {
        MSG("WARNING: Data type for clksrc seems wrong, please check\n");
        boardconf.clksrc = 0;
    }
    MSG("INFO: lorawan_public %d, clksrc %d\n", boardconf.lorawan_public, boardconf.clksrc);
    /* all parameters parsed, submitting configuration to the HAL */
    if (lgw_board_setconf(boardconf) != LGW_HAL_SUCCESS) {
        MSG("WARNING: Failed to configure board\n");
    }

    /* set configuration for RF chains */
    for (i = 0; i < LGW_RF_CHAIN_NB; ++i) {
        memset(&rfconf, 0, sizeof (rfconf)); /* initialize configuration structure */
        sprintf(param_name, "radio_%i", i); /* compose parameter path inside JSON structure */
        val = json_object_get_value(conf, param_name); /* fetch value (if possible) */
        if (json_value_get_type(val) != JSONObject) {
            MSG("INFO: no configuration for radio %i\n", i);
            continue;
        }
        /* there is an object to configure that radio, let's parse it */
        sprintf(param_name, "radio_%i.enable", i);
        val = json_object_dotget_value(conf, param_name);
        if (json_value_get_type(val) == JSONBoolean) {
            rfconf.enable = (bool) json_value_get_boolean(val);
        } else {
            rfconf.enable = false;
        }
        if (rfconf.enable == false) { /* radio disabled, nothing else to parse */
            MSG("INFO: radio %i disabled\n", i);
        } else { /* radio enabled, will parse the other parameters */
            snprintf(param_name, sizeof param_name, "radio_%i.freq", i);
            rfconf.freq_hz = (uint32_t) json_object_dotget_number(conf, param_name);
            snprintf(param_name, sizeof param_name, "radio_%i.rssi_offset", i);
            rfconf.rssi_offset = (float) json_object_dotget_number(conf, param_name);
            snprintf(param_name, sizeof param_name, "radio_%i.type", i);
            str = json_object_dotget_string(conf, param_name);
            if (!strncmp(str, "SX1255", 6)) {
                rfconf.type = LGW_RADIO_TYPE_SX1255;
            } else if (!strncmp(str, "SX1257", 6)) {
                rfconf.type = LGW_RADIO_TYPE_SX1257;
            } else {
                MSG("WARNING: invalid radio type: %s (should be SX1255 or SX1257)\n", str);
            }
            snprintf(param_name, sizeof param_name, "radio_%i.tx_enable", i);
            val = json_object_dotget_value(conf, param_name);
            if (json_value_get_type(val) == JSONBoolean) {
                rfconf.tx_enable = (bool) json_value_get_boolean(val);
            } else {
                rfconf.tx_enable = false;
            }
            MSG("INFO: radio %i enabled (type %s), center frequency %u, RSSI offset %f, tx enabled %d\n", i, str, rfconf.freq_hz, rfconf.rssi_offset, rfconf.tx_enable);
        }
        /* all parameters parsed, submitting configuration to the HAL */
        if (lgw_rxrf_setconf(i, rfconf) != LGW_HAL_SUCCESS) {
            MSG("WARNING: invalid configuration for radio %i\n", i);
        }
    }

    /* set configuration for LoRa multi-SF channels (bandwidth cannot be set) */
    for (i = 0; i < LGW_MULTI_NB; ++i) {
        memset(&ifconf, 0, sizeof (ifconf)); /* initialize configuration structure */
        sprintf(param_name, "chan_multiSF_%i", i); /* compose parameter path inside JSON structure */
        val = json_object_get_value(conf, param_name); /* fetch value (if possible) */
        if (json_value_get_type(val) != JSONObject) {
            MSG("INFO: no configuration for LoRa multi-SF channel %i\n", i);
            continue;
        }
        /* there is an object to configure that LoRa multi-SF channel, let's parse it */
        sprintf(param_name, "chan_multiSF_%i.enable", i);
        val = json_object_dotget_value(conf, param_name);
        if (json_value_get_type(val) == JSONBoolean) {
            ifconf.enable = (bool) json_value_get_boolean(val);
        } else {
            ifconf.enable = false;
        }
        if (ifconf.enable == false) { /* LoRa multi-SF channel disabled, nothing else to parse */
            MSG("INFO: LoRa multi-SF channel %i disabled\n", i);
        } else { /* LoRa multi-SF channel enabled, will parse the other parameters */
            sprintf(param_name, "chan_multiSF_%i.radio", i);
            ifconf.rf_chain = (uint32_t) json_object_dotget_number(conf, param_name);
            sprintf(param_name, "chan_multiSF_%i.if", i);
            ifconf.freq_hz = (int32_t) json_object_dotget_number(conf, param_name);
            // TODO: handle individual SF enabling and disabling (spread_factor)
            MSG("INFO: LoRa multi-SF channel %i enabled, radio %i selected, IF %i Hz, 125 kHz bandwidth, SF 7 to 12\n", i, ifconf.rf_chain, ifconf.freq_hz);
        }
        /* all parameters parsed, submitting configuration to the HAL */
        if (lgw_rxif_setconf(i, ifconf) != LGW_HAL_SUCCESS) {
            MSG("WARNING: invalid configuration for LoRa multi-SF channel %i\n", i);
        }
    }

    /* set configuration for LoRa standard channel */
    memset(&ifconf, 0, sizeof (ifconf)); /* initialize configuration structure */
    val = json_object_get_value(conf, "chan_Lora_std"); /* fetch value (if possible) */
    if (json_value_get_type(val) != JSONObject) {
        MSG("INFO: no configuration for LoRa standard channel\n");
    } else {
        val = json_object_dotget_value(conf, "chan_Lora_std.enable");
        if (json_value_get_type(val) == JSONBoolean) {
            ifconf.enable = (bool) json_value_get_boolean(val);
        } else {
            ifconf.enable = false;
        }
        if (ifconf.enable == false) {
            MSG("INFO: LoRa standard channel %i disabled\n", i);
        } else {
            ifconf.rf_chain = (uint32_t) json_object_dotget_number(conf, "chan_Lora_std.radio");
            ifconf.freq_hz = (int32_t) json_object_dotget_number(conf, "chan_Lora_std.if");
            bw = (uint32_t) json_object_dotget_number(conf, "chan_Lora_std.bandwidth");
            switch (bw) {
                case 500000: ifconf.bandwidth = BW_500KHZ;
                    break;
                case 250000: ifconf.bandwidth = BW_250KHZ;
                    break;
                case 125000: ifconf.bandwidth = BW_125KHZ;
                    break;
                default: ifconf.bandwidth = BW_UNDEFINED;
            }
            sf = (uint32_t) json_object_dotget_number(conf, "chan_Lora_std.spread_factor");
            switch (sf) {
                case 7: ifconf.datarate = DR_LORA_SF7;
                    break;
                case 8: ifconf.datarate = DR_LORA_SF8;
                    break;
                case 9: ifconf.datarate = DR_LORA_SF9;
                    break;
                case 10: ifconf.datarate = DR_LORA_SF10;
                    break;
                case 11: ifconf.datarate = DR_LORA_SF11;
                    break;
                case 12: ifconf.datarate = DR_LORA_SF12;
                    break;
                default: ifconf.datarate = DR_UNDEFINED;
            }
            MSG("INFO: LoRa standard channel enabled, radio %i selected, IF %i Hz, %u Hz bandwidth, SF %u\n", ifconf.rf_chain, ifconf.freq_hz, bw, sf);
        }
        if (lgw_rxif_setconf(8, ifconf) != LGW_HAL_SUCCESS) {
            MSG("WARNING: invalid configuration for LoRa standard channel\n");
        }
    }

    /* set configuration for FSK channel */
    memset(&ifconf, 0, sizeof (ifconf)); /* initialize configuration structure */
    val = json_object_get_value(conf, "chan_FSK"); /* fetch value (if possible) */
    if (json_value_get_type(val) != JSONObject) {
        MSG("INFO: no configuration for FSK channel\n");
    } else {
        val = json_object_dotget_value(conf, "chan_FSK.enable");
        if (json_value_get_type(val) == JSONBoolean) {
            ifconf.enable = (bool) json_value_get_boolean(val);
        } else {
            ifconf.enable = false;
        }
        if (ifconf.enable == false) {
            MSG("INFO: FSK channel %i disabled\n", i);
        } else {
            ifconf.rf_chain = (uint32_t) json_object_dotget_number(conf, "chan_FSK.radio");
            ifconf.freq_hz = (int32_t) json_object_dotget_number(conf, "chan_FSK.if");
            bw = (uint32_t) json_object_dotget_number(conf, "chan_FSK.bandwidth");
            if (bw <= 7800) ifconf.bandwidth = BW_7K8HZ;
            else if (bw <= 15600) ifconf.bandwidth = BW_15K6HZ;
            else if (bw <= 31200) ifconf.bandwidth = BW_31K2HZ;
            else if (bw <= 62500) ifconf.bandwidth = BW_62K5HZ;
            else if (bw <= 125000) ifconf.bandwidth = BW_125KHZ;
            else if (bw <= 250000) ifconf.bandwidth = BW_250KHZ;
            else if (bw <= 500000) ifconf.bandwidth = BW_500KHZ;
            else ifconf.bandwidth = BW_UNDEFINED;
            ifconf.datarate = (uint32_t) json_object_dotget_number(conf, "chan_FSK.datarate");
            MSG("INFO: FSK channel enabled, radio %i selected, IF %i Hz, %u Hz bandwidth, %u bps datarate\n", ifconf.rf_chain, ifconf.freq_hz, bw, ifconf.datarate);
        }
        if (lgw_rxif_setconf(9, ifconf) != LGW_HAL_SUCCESS) {
            MSG("WARNING: invalid configuration for FSK channel\n");
        }
    }
    json_value_free(root_val);
    return 0;
}

int parse_gateway_configuration(const char * conf_file) {
    const char conf_obj[] = "gateway_conf";
    JSON_Value *root_val;
    JSON_Object *root = NULL;
    JSON_Object *conf = NULL;
    const char *str; /* pointer to sub-strings in the JSON data */
    unsigned long long ull = 0;

    /* try to parse JSON */
    root_val = json_parse_file_with_comments(conf_file);
    root = json_value_get_object(root_val);
    if (root == NULL) {
        MSG("ERROR: %s id not a valid JSON file\n", conf_file);
        exit(EXIT_FAILURE);
    }
    conf = json_object_get_object(root, conf_obj);
    if (conf == NULL) {
        MSG("INFO: %s does not contain a JSON object named %s\n", conf_file, conf_obj);
        return -1;
    } else {
        MSG("INFO: %s does contain a JSON object named %s, parsing gateway parameters\n", conf_file, conf_obj);
    }

    /* getting network parameters (only those necessary for the packet logger) */
    str = json_object_get_string(conf, "gateway_ID");
    if (str != NULL) {
        sscanf(str, "%llx", &ull);
        lgwm = ull;
        MSG("INFO: gateway MAC address is configured to %016llX\n", ull);
    }

    json_value_free(root_val);
    return 0;
}

/* describe command line options */
void usage(void) {
    printf("*** Library version information ***\n%s\n\n", lgw_version_info());
    printf("Available options:\n");
    printf(" -h print this help\n");
    printf(" -r <int> rotate log file every N seconds (-1 disable log rotation)\n");
}

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
        double RSSI,
        time TIMESTAMP,
        uint32 BAD_WIDTH,
        uint32 CODE_RATE,
        uint32 SF,
        uint16 SIZE,
        uint8 RF_CHAIN,
        double SNR,
        uint64 DEV_ADDR,
        uint32 FRQ,
        uint32 US_COUNT,
        uint8 STATUS,
        uint8 MOD,
        uint16 FCNT,
        uint8 MS_TYPE,
        string PHY_PAYLOAD,
        string APP_EUI,
        string DEV_EUI,
        string FOPTS,
        string FPORT,
        string DEV_NONCE,
        string FCTRL,
        string FHDR,
        string APP_NONCE,
        string MHDR,
        string MIC,
        string NET_ID
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
    PARAM('d', "debug", "Set debugging", no_argument, "none")
    
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
    fprintf(stderr, arg);
    TRAP_DEFAULT_FINALIZATION();
}

/** ---- MAIN ----- */
int main(int argc, char **argv) {
    /** SectionFields LoRa logger */
    int i, j, g; /* loop and temporary variables */
    struct timespec sleep_time = {0, 3000000}; /* 3 ms */

    char buff[3];
    char payload[512]; /** Maximale payload size */

    /* clock and log rotation management */
    int log_rotate_interval = 3600; /* by default, rotation every hour */
    int time_check = 0; /* variable used to limit the number of calls to time() function */

    /* configuration file related */
    const char global_conf_fname[] = "global_conf.json"; /* contain global (typ. network-wide) configuration */
    const char local_conf_fname[] = "local_conf.json"; /* contain node specific configuration, overwrite global parameters for parameters that are defined in both */
    const char debug_conf_fname[] = "debug_conf.json"; /* if present, all other configuration files are ignored */

    /* allocate memory for packet fetching and processing */
    struct lgw_pkt_rx_s rxpkt[16]; /* array containing up to 16 inbound packets metadata */
    struct lgw_pkt_rx_s *p; /* pointer on a RX packet */
    int nb_pkt;

    /* local timestamp variables until we get accurate GPS time */
    struct timespec fetch_time;
    char fetch_timestamp[30];
    struct tm * x;

    int ret;
    signed char opt;
    /** endSection */
    
    
    /* **** TRAP initialization **** */

    /*
     * Macro allocates and initializes module_info structure according to MODULE_BASIC_INFO and MODULE_PARAMS
     * definitions on the lines 470 and 478 of this file. It also creates a string with short_opt letters for getopt
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
            case 'd':
                debug = 1;
                break;
            default:
                trap_fin("Invalid arguments.\n");
                FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS);
                return -1;
        }
    }

    /** Create Output UniRec templates */
    ur_template_t *out_tmplt = ur_create_output_template(0, "RSSI,TIMESTAMP,BAD_WIDTH,CODE_RATE,SF,SIZE,RF_CHAIN,SNR,DEV_ADDR,FRQ,US_COUNT,STATUS,MOD,FCNT,MS_TYPE,PHY_PAYLOAD,APP_EUI,DEV_EUI,FOPTS,FPORT,DEV_NONCE,FCTRL,FHDR,APP_NONCE,MHDR,MIC,NET_ID", NULL);
    if (out_tmplt == NULL) {
        ur_free_template(out_tmplt);
        fprintf(stderr, "Error: Output template could not be created.\n");
        return -1;
    }

    /** Allocate memory for output record */
    void *out_rec = ur_create_record(out_tmplt, MAX_MSG_SIZE);
    if (out_rec == NULL) {
        //        ur_free_template(in_tmplt);
        ur_free_template(out_tmplt);
        ur_free_record(out_rec);
        fprintf(stderr, "Error: Memory allocation problem (output record).\n");
        return -1;
    }
    

    /* configure signal handling */
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = sig_handler;
    sigaction(SIGQUIT, &sigact, NULL);
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);

    /* configuration files management */
    if (access(debug_conf_fname, R_OK) == 0) {
        /* if there is a debug conf, parse only the debug conf */
        MSG("INFO: found debug configuration file %s, other configuration files will be ignored\n", debug_conf_fname);
        parse_SX1301_configuration(debug_conf_fname);
        parse_gateway_configuration(debug_conf_fname);
    } else if (access(global_conf_fname, R_OK) == 0) {
        /* if there is a global conf, parse it and then try to parse local conf  */
        MSG("INFO: found global configuration file %s, trying to parse it\n", global_conf_fname);
        parse_SX1301_configuration(global_conf_fname);
        parse_gateway_configuration(global_conf_fname);
        if (access(local_conf_fname, R_OK) == 0) {
            MSG("INFO: found local configuration file %s, trying to parse it\n", local_conf_fname);
            parse_SX1301_configuration(local_conf_fname);
            parse_gateway_configuration(local_conf_fname);
        }
    } else if (access(local_conf_fname, R_OK) == 0) {
        /* if there is only a local conf, parse it and that's all */
        MSG("INFO: found local configuration file %s, trying to parse it\n", local_conf_fname);
        parse_SX1301_configuration(local_conf_fname);
        parse_gateway_configuration(local_conf_fname);
    } else {
        MSG("ERROR: failed to find any configuration file named %s, %s or %s\n", global_conf_fname, local_conf_fname, debug_conf_fname);
        return EXIT_FAILURE;
    }

    /* starting the concentrator */
    i = lgw_start();
    if (i == LGW_HAL_SUCCESS) {
        MSG("INFO: concentrator started, packet can now be received\n");
    } else {
        MSG("ERROR: failed to start the concentrator\n");
        return EXIT_FAILURE;
    }

    /* transform the MAC address into a string */
    sprintf(lgwm_str, "%08X%08X", (uint32_t) (lgwm >> 32), (uint32_t) (lgwm & 0xFFFFFFFF));

 
    payload[0] = '\0';
    while ((quit_sig != 1) && (exit_sig != 1) && (!stop)) {
        /* fetch packets */
        nb_pkt = lgw_receive(ARRAY_SIZE(rxpkt), rxpkt);
        if (nb_pkt == LGW_HAL_ERROR) {
            MSG("ERROR: failed packet fetch, exiting\n");
            return EXIT_FAILURE;
        } else if (nb_pkt == 0) {
            clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep_time, NULL); /* wait a short time if no packets */
        } else {
            /* local timestamp generation until we get accurate GPS time */
            clock_gettime(CLOCK_REALTIME, &fetch_time);
            x = gmtime(&(fetch_time.tv_sec));
            sprintf(fetch_timestamp, "%04i-%02i-%02i %02i:%02i:%02i.%03liZ", (x->tm_year) + 1900, (x->tm_mon) + 1, x->tm_mday, x->tm_hour, x->tm_min, x->tm_sec, (fetch_time.tv_nsec) / 1000000); /* ISO 8601 format */
        }

        /* log packets */
        for (i = 0; i < nb_pkt; ++i) {
            p = &rxpkt[i];

            /* writing bandwidth */
            uint32_t band_width = -1;
            switch (p->bandwidth) {
                case BW_500KHZ: band_width = 500000;
                    break;
                case BW_250KHZ: band_width = 250000;
                    break;
                case BW_125KHZ: band_width = 125000;
                    break;
                case BW_62K5HZ: band_width = 62500;
                    break;
                case BW_31K2HZ: band_width = 31200;
                    break;
                case BW_15K6HZ: band_width = 15600;
                    break;
                case BW_7K8HZ: band_width = 7800;
                    break;
                case BW_UNDEFINED: band_width = 0;
                    break;
                default: band_width = -1;
            }

            /* writing datarate */
            uint32_t sf = -1;
            if (p->modulation == MOD_LORA) {
                switch (p->datarate) {
                    case DR_LORA_SF7: sf = 7;
                        break;
                    case DR_LORA_SF8: sf = 8;
                        break;
                    case DR_LORA_SF9: sf = 9;
                        break;
                    case DR_LORA_SF10: sf = 10;
                        break;
                    case DR_LORA_SF11: sf = 11;
                        break;
                    case DR_LORA_SF12: sf = 12;
                        break;
                    default: sf = -1;
                }
            } else if (p->modulation == MOD_FSK) {
                sf = p->datarate;
            } else {
                sf = -1;
            }

            /* writing coderate */
            uint32_t code_rate = -1;
            switch (p->coderate) {
                case CR_LORA_4_5: code_rate = 5;
                    break;
                case CR_LORA_4_6: code_rate = 6;
                    break;
                case CR_LORA_4_7: code_rate = 7;
                    break;
                case CR_LORA_4_8: code_rate = 8;
                    break;
                case CR_UNDEFINED: code_rate = 0;
                    break;
                default: code_rate = -1;
            }

            /* writing payload to char */
            for (g = 0; g < p->size; ++g) {
                if (g > 0){
			sprintf(buff, "%02X", p->payload[g]);
                	buff[2] = '\0';
                	strcat(payload, buff);
		}
            }

	    /** Timestamp time */
	    time_t t = time(0);
	    ur_time_t timestamp = ur_time_from_sec_msec(t, t/1000);
            
            /** Check size payload min/max */
            if (p->size < 14 || p->size > 512){
                payload[0] = '\0';
                lr_free();
                continue;
            }

	    /** Initialization physical payload for parsing and reversing octet fields. */
            lr_initialization(payload);
	    
	    if (DevAddr != NULL){
		uint64_t dev_addr = lr_uint8_to_uint64(lr_arr_to_uint8(lr_revers_array(DevAddr)));
		ur_set(out_tmplt, out_rec, F_DEV_ADDR, dev_addr);
	    }

            /* Set value to UniRec fields */
            ur_set(out_tmplt, out_rec, F_RSSI, (double) p->rssi);
            ur_set(out_tmplt, out_rec, F_TIMESTAMP, timestamp);
	    ur_set(out_tmplt, out_rec, F_BAD_WIDTH, band_width);
	    ur_set(out_tmplt, out_rec, F_CODE_RATE, code_rate);
            ur_set(out_tmplt, out_rec, F_SF, sf);
	    ur_set(out_tmplt, out_rec, F_SIZE, p->size);
            ur_set(out_tmplt, out_rec, F_RF_CHAIN, p->rf_chain);
            ur_set(out_tmplt, out_rec, F_SNR, (double) p->snr);
	    ur_set(out_tmplt, out_rec, F_FRQ, p->freq_hz);
	    ur_set(out_tmplt, out_rec, F_US_COUNT, p->count_us);
	    ur_set(out_tmplt, out_rec, F_STATUS, p->status);
	    ur_set(out_tmplt, out_rec, F_MOD, p->modulation);
	    ur_set_string(out_tmplt, out_rec, F_PHY_PAYLOAD, payload);

            
            /* Set parsing value to UniRec fields */
            if(AppEUI != NULL)
 	    	ur_set_string(out_tmplt, out_rec, F_APP_EUI, AppEUI);
            if(DevEUI != NULL)
	    	ur_set_string(out_tmplt, out_rec, F_DEV_EUI, DevEUI);
    	    if(FOpts != NULL)
            	ur_set_string(out_tmplt, out_rec, F_FOPTS, FOpts);
            if(FPort != NULL)
	    	ur_set_string(out_tmplt, out_rec, F_FPORT, FPort);
            if(DevNonce != NULL)
	    	ur_set_string(out_tmplt, out_rec, F_DEV_NONCE, DevNonce);
            if(FCtrl != NULL)
	    	ur_set_string(out_tmplt, out_rec, F_FCTRL, FCtrl);
            if(FHDR != NULL)
	    	ur_set_string(out_tmplt, out_rec, F_FHDR, FHDR);
            if(AppNonce != NULL)
	    	ur_set_string(out_tmplt, out_rec, F_APP_NONCE, AppNonce);
            if(MHDR != NULL){
	    	ur_set_string(out_tmplt, out_rec, F_MHDR, MHDR);
                ur_set(out_tmplt, out_rec, F_MS_TYPE, lr_get_message_type());
            };
            if(MIC != NULL)
	    	ur_set_string(out_tmplt, out_rec, F_MIC, MIC);
            if(NetID != NULL)
	    	ur_set_string(out_tmplt, out_rec, F_NET_ID, NetID);
            if(FCnt != NULL){
                uint16_t fcnt = lr_arr_to_uint16(FCnt);
                ur_set(out_tmplt, out_rec, F_FCNT, fcnt);
            };

            /** Counter for status packet */
            if(debug){
                (p->status == 16) ? st_counter.cnt_pkt_log++ : st_counter.cnt_bad_pkt_log++;
                st_counter.cnt_all_pkt_log = st_counter.cnt_pkt_log + st_counter.cnt_bad_pkt_log;
            }
            
            /** Debugging message */
            MSG("------------------------------\n");
            MSG("MESSAGE: Status packet -> %s\n", (p->status == 16) ? "OK" : "BAD");
            MSG("MESSAGE: Size unirec template -> %d\n", ur_rec_size(out_tmplt, out_rec));
            MSG("MESSAGE: Device address -> %s\n", DevAddr);
            MSG("MESSAGE: Physical payload -> %s\n", payload);
            MSG("MESSGAE: Total status packet OK -> %d, BAD -> %d, ALL -> %d\n", st_counter.cnt_pkt_log, st_counter.cnt_bad_pkt_log, st_counter.cnt_all_pkt_log);
            MSG("------------------------------\n");

            
            /* send data */
            ret = trap_send(0, out_rec, ur_rec_size(out_tmplt, out_rec));
            payload[0] = '\0';
	    lr_free();
            
            TRAP_DEFAULT_SEND_ERROR_HANDLING(ret, continue, break);
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
    //    ur_free_template(in_tmplt);
    ur_free_template(out_tmplt);
    ur_finalize();

    /**
     * Free logger 
     */
    i = lgw_stop();

    return 0;
}

