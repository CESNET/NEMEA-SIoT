#ifndef _UR_FIELDS_H_
#define _UR_FIELDS_H_

/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
#include <unirec/unirec.h>

#define F_TIMESTAMP   0
#define F_TIMESTAMP_T   ur_time_t        
#define F_AIR_TIME   1
#define F_AIR_TIME_T   double_t
#define F_DEV_ADDR   2
#define F_DEV_ADDR_T   uint64_t
#define F_INCIDENT_DEV_ADDR   3
#define F_INCIDENT_DEV_ADDR_T   uint64_t
#define F_BAD_WIDTH   4
#define F_BAD_WIDTH_T   uint32_t
#define F_SF   5
#define F_SF_T   uint32_t
#define F_CODE_RATE   6
#define F_CODE_RATE_T   uint32_t
#define F_ALERT_CODE   7
#define F_ALERT_CODE_T   uint32_t
#define F_SIZE   8
#define F_SIZE_T   uint16_t         
#define F_STATUS   9
#define F_STATUS_T   uint8_t
#define F_MS_TYPE   10
#define F_MS_TYPE_T   uint8_t
#define F_CAPTION   11
#define F_CAPTION_T   char

extern uint16_t ur_last_id;
extern ur_static_field_specs_t UR_FIELD_SPECS_STATIC;
extern ur_field_specs_t ur_field_specs;

#endif

