/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   packet.h
 * Author: root
 *
 * Created on August 29, 2017, 9:12 AM
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef PACKET_H
#define PACKET_H

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

    int getMessageType();
    void initialization(char* packet);
    
    int getInt(char *arr);

    bool isDataMessage();
    bool isJoinRequestMessage();
    bool isJoinAcceptMessage();

    char *reversArray(char *arr);
    char *slice(char *arr, size_t start, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* PACKET_H */
