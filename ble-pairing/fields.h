#ifndef _UR_FIELDS_H_
#define _UR_FIELDS_H_

/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
#include <unirec/unirec.h>

#define F_TIMESTAMP   0
#define F_TIMESTAMP_T   ur_time_t
#define F_DEV_ADDR   1
#define F_DEV_ADDR_T   mac_addr_t
#define F_HCI_DEV_ADDR   2
#define F_HCI_DEV_ADDR_T   mac_addr_t
#define F_INCIDENT_DEV_ADDR   3
#define F_INCIDENT_DEV_ADDR_T   mac_addr_t
#define F_ALERT_CODE   4
#define F_ALERT_CODE_T   uint32_t
#define F_SIZE   5
#define F_SIZE_T   uint16_t
#define F_DATA_DIRECTION   6
#define F_DATA_DIRECTION_T   uint8_t
#define F_METHOD   7
#define F_METHOD_T   uint8_t
#define F_PACKET_TYPE   8
#define F_PACKET_TYPE_T   uint8_t
#define F_SUCCESS   9
#define F_SUCCESS_T   uint8_t
#define F_VERSION   10
#define F_VERSION_T   uint8_t
#define F_CAPTION   11
#define F_CAPTION_T   char
#define F_PACKET   12
#define F_PACKET_T   char

extern uint16_t ur_last_id;
extern ur_static_field_specs_t UR_FIELD_SPECS_STATIC;
extern ur_field_specs_t ur_field_specs;

#endif

