#ifndef _UR_FIELDS_H_
#define _UR_FIELDS_H_

/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
#include <unirec/unirec.h>

#define F_RSSI   0
#define F_RSSI_T   double
#define F_TIMESTAMP   1
#define F_TIMESTAMP_T   ur_time_t
#define F_BAD_WIDTH   2
#define F_BAD_WIDTH_T   uint32_t
#define F_CODE_RATE   3
#define F_CODE_RATE_T   uint32_t
#define F_SF   4
#define F_SF_T   uint32_t
#define F_SIZE   5
#define F_SIZE_T   uint32_t
#define F_RF_CHAIN  6
#define F_RF_CHAIN_T uint32_t
#define F_SNR   7
#define F_SNR_T double
#define F_DEV_ADDR  8
#define F_DEV_ADDR_T uint64_t
#define F_FRQ   9
#define F_FRQ_T uint32_t
#define F_US_COUNT  10
#define F_US_COUNT_T    uint32_t
#define F_STATUS    11
#define F_STATUS_T  uint8_t
#define F_MOD   12
#define F_MOD_T uint8_t
#define F_PHY_PAYLOAD   13
#define F_PHY_PAYLOAD_T   char
#define F_APP_EUI   14
#define F_APP_EUI_T char
#define F_DEV_EUI   15
#define F_DEV_EUI_T char
#define F_FOPTS    16
#define F_FOPTS_T   char
#define F_FPORT 17
#define F_FPORT_T   char
#define F_DEV_NONCE 18
#define F_DEV_NONCE_T   char
#define F_FCTRL 19
#define F_FCTRL_T   char
#define F_FHDR  20
#define F_FHDR_T    char
#define F_APP_NONCE 21
#define F_APP_NONCE_T   char
#define F_MHDR  22
#define F_MHDR_T    char
#define F_MIC   23
#define F_MIC_T char
#define F_NET_ID    24
#define F_NET_ID_T  char

extern uint16_t ur_last_id;
extern ur_static_field_specs_t UR_FIELD_SPECS_STATIC;
extern ur_field_specs_t ur_field_specs;

#endif

