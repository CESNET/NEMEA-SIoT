#ifndef _UR_FIELDS_H_
#define _UR_FIELDS_H_

/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
#include <unirec/unirec.h>

#define F_EVENT_TYPE   0
#define F_EVENT_TYPE_T   double
#define F_HOME_ID   1
#define F_HOME_ID_T   double
#define F_INCIDENT_DEV_ADDR   2
#define F_INCIDENT_DEV_ADDR_T   uint64_t
#define F_NODE_ID   3
#define F_NODE_ID_T   double
#define F_TIME   4
#define F_TIME_T   ur_time_t
#define F_TIMESTAMP   5
#define F_TIMESTAMP_T   ur_time_t
#define F_ALERT_CODE   6
#define F_ALERT_CODE_T   uint32_t
#define F_CHANNEL   7
#define F_CHANNEL_T   uint8_t
#define F_CAPTION   8
#define F_CAPTION_T   char
#define F_FRAME   9
#define F_FRAME_T   char

extern uint16_t ur_last_id;
extern ur_static_field_specs_t UR_FIELD_SPECS_STATIC;
extern ur_field_specs_t ur_field_specs;

#endif

