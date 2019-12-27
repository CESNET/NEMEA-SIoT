#ifndef _UR_FIELDS_H_
#define _UR_FIELDS_H_

/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
#include <unirec/unirec.h>

#define F_BASE_RSSI   0
#define F_BASE_RSSI_T   double
#define F_DEV_ADDR   1
#define F_DEV_ADDR_T   uint64_t
#define F_INCIDENT_DEV_ADDR   2
#define F_INCIDENT_DEV_ADDR_T   uint64_t
#define F_RSSI   3
#define F_RSSI_T   double
#define F_TIMESTAMP   4
#define F_TIMESTAMP_T   ur_time_t
#define F_VARIANCE   5
#define F_VARIANCE_T   double
#define F_ALERT_CODE   6
#define F_ALERT_CODE_T   uint32_t
#define F_CAPTION   7
#define F_CAPTION_T   char
#define F_PHY_PAYLOAD   8
#define F_PHY_PAYLOAD_T   char

extern uint16_t ur_last_id;
extern ur_static_field_specs_t UR_FIELD_SPECS_STATIC;
extern ur_field_specs_t ur_field_specs;

#endif

