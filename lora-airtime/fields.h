#ifndef _UR_FIELDS_H_
#define _UR_FIELDS_H_

/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
#include <unirec/unirec.h>

#define F_AIR_TIME   0
#define F_AIR_TIME_T   uint64_t
#define F_TIMESTAMP   1
#define F_TIMESTAMP_T   uint64_t
#define F_BAD_WIDTH   2
#define F_BAD_WIDTH_T   uint32_t
#define F_CODE_RATE   3
#define F_CODE_RATE_T   uint32_t
#define F_SF   4
#define F_SF_T   uint32_t
#define F_SIZE   5
#define F_SIZE_T   uint32_t
#define F_ENABLE   6
#define F_ENABLE_T   uint8_t
#define F_APP_SKEY   7
#define F_APP_SKEY_T   char
#define F_DEV_ADDR   8
#define F_DEV_ADDR_T   char
#define F_NWK_SKEY   9
#define F_NWK_SKEY_T   char
#define F_PHY_PAYLOAD   10
#define F_PHY_PAYLOAD_T   char

extern uint16_t ur_last_id;
extern ur_static_field_specs_t UR_FIELD_SPECS_STATIC;
extern ur_field_specs_t ur_field_specs;

#endif

