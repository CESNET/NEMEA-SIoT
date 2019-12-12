#ifndef _UR_FIELDS_H_
#define _UR_FIELDS_H_

/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
#include <unirec/unirec.h>

#define F_DEV_ADDR   0
#define F_DEV_ADDR_T   uint64_t
#define F_TIMESTAMP   1
#define F_TIMESTAMP_T   ur_time_t
#define F_HOME_ID   2
#define F_HOME_ID_T   uint32_t
#define F_ACK_REQ   3
#define F_ACK_REQ_T   uint8_t
#define F_CHANNEL   4
#define F_CHANNEL_T   uint8_t
#define F_DST_HOP   5
#define F_DST_HOP_T   uint8_t
#define F_DST_ID   6
#define F_DST_ID_T   uint8_t
#define F_FAILED_HOP   7
#define F_FAILED_HOP_T   uint8_t
#define F_ROUTED   8
#define F_ROUTED_T   uint8_t
#define F_SEQ_NUM   9
#define F_SEQ_NUM_T   uint8_t
#define F_SIZE   10
#define F_SIZE_T   uint8_t
#define F_SRC_HOP   11
#define F_SRC_HOP_T   uint8_t
#define F_CMD_CLASS_STR   12
#define F_CMD_CLASS_STR_T   char
#define F_CMD_STR   13
#define F_CMD_STR_T   char
#define F_FRAME   14
#define F_FRAME_T   char
#define F_HOPS   15
#define F_HOPS_T   char
#define F_PAYLOAD   16
#define F_PAYLOAD_T   char
#define F_ROUTED_TYPE   17
#define F_ROUTED_TYPE_T   char
#define F_TYPE   18
#define F_TYPE_T   char

extern uint16_t ur_last_id;
extern ur_static_field_specs_t UR_FIELD_SPECS_STATIC;
extern ur_field_specs_t ur_field_specs;

#endif

