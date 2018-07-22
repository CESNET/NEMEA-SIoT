/* 
 * File:   lora_packet.h
 * Author: Erik Gresak
 * Email: gre0071@vsb.cz
 * Created on March 21, 2018, 9:12 AM
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "aes/aes.h"

#ifndef LORA_PACKET_H
#define LORA_PACKET_H



#ifdef __cplusplus
extern "C" {
#endif

    char *AppEUI;
    char *DevEUI;
    char *DevNonce;
    char *AppNonce;
    char *NetID;
    char *CFList;
    char *PHYPayload;
    char *MHDR;
    char *MACPayload;
    char *MIC;
    char *FCtrl;
    char *FOpts;
    char *FHDR;
    char *DevAddr;
    char *FCnt;
    char *FPort;
    char *FRMPayload;


    int i_FCtrl;
    int FOptsLen;
    int DLSettings;
    int RxDelay;

    void lr_initialization(char* packet);
    void lr_free();

    int lr_get_int(char *arr);
    int lr_get_message_type();
    uint8_t lr_get_direction();

    bool lr_is_data_message();
    bool lr_is_join_request_message();
    bool lr_is_join_accept_message();
    
    double lr_airtime_calculate(unsigned int pay_size, uint8_t header, uint8_t dr, unsigned int sf, unsigned int cd_rate, unsigned int prem_sym, unsigned int band, double duty_cycle);

    char *lr_revers_array(char *arr);
    char *lr_slice(char *arr, size_t start, size_t size);
    uint8_t *lr_arr_to_uint8(char* arr);
    uint16_t lr_arr_to_uint16(char* arr);
    uint64_t lr_uint8_to_uint64(uint8_t* var);
    void lr_print_uint8(uint8_t* str);
    char *lr_uint8_to_string(uint8_t* arr);

    uint8_t *lr_decode(uint8_t* nwkSKey, uint8_t* appSKey);

#ifdef __cplusplus
}
#endif

#endif /* LORA_PACKET_H */

