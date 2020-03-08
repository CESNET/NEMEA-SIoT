#ifndef _UR_FIELDS_H_
#define _UR_FIELDS_H_

/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
#include <unirec/unirec.h>

#define F_TIMESTAMP   0
#define F_TIMESTAMP_T   ur_time_t
#define F_DEV_ADDR   1
#define F_DEV_ADDR_T   uint64_t
#define F_INCIDENT_DEV_ADDR   2
#define F_INCIDENT_DEV_ADDR_T   uint64_t
#define F_ALERT_CODE   3
#define F_ALERT_CODE_T   uint32_t
#define F_FCNT   4
#define F_FCNT_T   uint16_t
#define F_STATUS   5
#define F_STATUS_T   uint8_t
#define F_MS_TYPE   6
#define F_MS_TYPE_T   uint8_t
#define F_CAPTION   7
#define F_CAPTION_T   char

extern uint16_t ur_last_id;
extern ur_static_field_specs_t UR_FIELD_SPECS_STATIC;
extern ur_field_specs_t ur_field_specs;

#endif

