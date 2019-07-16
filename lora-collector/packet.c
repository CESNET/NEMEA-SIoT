/* 
 * File:   packet.c
 * Author: Erik Gresak
 * Email: erik.g@seznam.cz
 * Created on August 23, 2017, 7:26 AM
 */




#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "packet.h"

// Sector Constant
// decoding MType description & direction
#define MTYPE_JOIN_REQUEST 0
#define MTYPE_JOIN_ACCEPT 1
#define MTYPE_UNCONFIRMED_DATA_UP 2
#define MTYPE_UNCONFIRMED_DATA_DOWN 3
#define MTYPE_CONFIRMED_DATA_UP 4
#define MTYPE_CONFIRMED_DATA_DOWN 5

// endSector

char *slice(char *arr, size_t start, size_t size) {
    if (arr == NULL)
        return NULL;

    char *_arr = (char*) malloc(size + 1);

    for (int i = start; i < size + start; i++) {
        _arr[i - start] = arr[i];
    }

    _arr[size] = '\0';

    return _arr;
}

char *reversArray(char *arr) {
    if (arr == NULL)
        return NULL;

    int end = strlen(arr);
    int len = strlen(arr);

    char *_array = (char*) malloc(len + 1);

    for (int i = 0; i < len; i++) {
        --end;
        if ((i % 2) == 0) {
            _array[i] = arr[end - 1];
        } else {
            _array[i] = arr[end + 1];
        }
    }

    _array[len] = '\0';

    return _array;   
}

int getInt(char *arr) {
    char hexDigits[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    char *_i = (char*) malloc(strlen(arr));
    int len = strlen(arr);
    int j, decimal = 0;

    for (int i = 0; i < len; i++) {
        _i[i] = arr[i];
    }

    for (int i = 0; i < len; i++) {
        for (j = 0; j < 16; j++) {
            if (_i[i] == hexDigits[j]) {
                decimal += j * (int) pow(16, (len-1) - i);
            }
        }
    }
    
    free(_i);
    _i = NULL;
    
    return decimal;
}

int getMessageType() {
    return (getInt(MHDR) & 0xff) >> 5; //5
}

bool isDataMessage() {
    switch (getMessageType()) {
        case MTYPE_UNCONFIRMED_DATA_UP:
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_UP:
        case MTYPE_CONFIRMED_DATA_DOWN:
            return true;
        default:
            return false;
    }
}

bool isJoinRequestMessage() {
    return (getMessageType() == MTYPE_JOIN_REQUEST);
}

bool isJoinAcceptMessage() {
    return (getMessageType() == MTYPE_JOIN_ACCEPT);
}

void freeMem() {
    free(MHDR);
    free(FCtrl);
    free(FCnt);
    free(FRMPayload);
    free(FPort);
    free(AppEUI);
    free(DevEUI);
    free(DevNonce);
    free(MIC);
    free(AppNonce);
    free(NetID);
    free(CFList);
    free(MACPayload);
    free(FOpts);
    free(FHDR);

    MHDR = NULL;
    FCtrl = NULL;
    FCnt = NULL;
    FRMPayload = NULL;
    FPort = NULL;
    AppEUI = NULL;
    DevEUI = NULL;
    DevNonce = NULL;
    MIC = NULL;
    AppNonce = NULL;
    NetID = NULL;
    CFList = NULL;
    MACPayload = NULL;
    FOpts = NULL;
    FHDR = NULL;
}

// Sector initialization

void initialization(char* packet) {
    size_t _strl = strlen(packet);
    char *_packet = packet;

    // initialization
    MHDR = (char*) malloc(2);
    MIC = (char*) malloc(8);
    FCtrl = (char*) malloc(2);
    DevAddr = (char*) malloc(8);
    FCnt = (char*) malloc(4);
    FRMPayload = (char*) malloc(0);
    FPort = (char*) malloc(0);

    AppEUI = (char*) malloc(16);
    DevEUI = (char*) malloc(16);
    DevNonce = (char*) malloc(4);
    MIC = (char*) malloc(8);

    AppNonce = (char*) malloc(6);
    NetID = (char*) malloc(6);
    DevAddr = (char*) malloc(8);
    CFList = (char*) malloc(0);

    PHYPayload = packet;
    MHDR = slice(_packet, 0, 2);

    if (isJoinRequestMessage()) {
        AppEUI = reversArray(slice(_packet, 2, 16));
        DevEUI = reversArray(slice(_packet, 18, 16));
        DevNonce = reversArray(slice(_packet, 34, 4));
        MIC = slice(_packet, _strl - 8, 8);
    } else if (isJoinAcceptMessage()) {
        AppNonce = reversArray(slice(_packet, 2, 6));
        NetID = reversArray(slice(_packet, 8, 6));
        DevAddr = reversArray(slice(_packet, 14, 8));
        DLSettings = getInt(_packet);
        RxDelay = getInt(_packet);
        if (_strl == 66) {
            CFList = (char*) malloc(32);
            CFList = slice(_packet, 26, 32);
        }
        MIC = slice(_packet, _strl - 8, 8);
    } else if (isDataMessage() && (_strl > 10)) {


        MACPayload = (char*) malloc(_strl - 10);

        MACPayload = slice(_packet, 2, (_strl - 10));
        MIC = slice(_packet, (_strl - 8), 8);

        FCtrl = slice(MACPayload, 8, 2);

        int _FCtrl = getInt(FCtrl);
        int FOptsLen = (_FCtrl & 0x0f) *2;

        if (FOptsLen != 0) {
            FOpts = (char*) malloc(FOptsLen);
            FOpts = slice(MACPayload, 14, FOptsLen);
        }



        int FHDR_length = 14 + FOptsLen;
        FHDR = (char*) malloc(FHDR_length);
        FHDR = slice(MACPayload, 0, 0 + FHDR_length);
        DevAddr = reversArray(slice(FHDR, 0, 8));

        FCnt = slice(FHDR, 10, 4);
        FCnt = reversArray(FCnt);

        if (FHDR_length != strlen(MACPayload)) {
            FPort = (char*) malloc(2);
            FPort = slice(MACPayload, FHDR_length, 2);

            if (FHDR_length < strlen(MACPayload)) {
                FRMPayload = (char*) malloc(strlen(MACPayload)-(FHDR_length + 2));
                FRMPayload = slice(MACPayload, FHDR_length + 2, strlen(MACPayload) - (FHDR_length + 2));
            }
        }

    }

}
// endSector

