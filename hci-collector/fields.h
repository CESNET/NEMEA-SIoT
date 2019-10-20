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
#define F_SIZE   3
#define F_SIZE_T   uint16_t
#define F_DATA_DIRECTION   4
#define F_DATA_DIRECTION_T   uint8_t
#define F_PACKET_TYPE   5
#define F_PACKET_TYPE_T   uint8_t
#define F_PACKET   6
#define F_PACKET_T   char

extern uint16_t ur_last_id;
extern ur_static_field_specs_t UR_FIELD_SPECS_STATIC;
extern ur_field_specs_t ur_field_specs;

#endif

