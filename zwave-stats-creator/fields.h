#ifndef _UR_FIELDS_H_
#define _UR_FIELDS_H_

/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
#include <unirec/unirec.h>

#define F_ACK_C   0
#define F_ACK_C_T   double
#define F_BROADCAST_C   1
#define F_BROADCAST_C_T   double
#define F_CORRUPTED_C   2
#define F_CORRUPTED_C_T   double
#define F_CORRUPTED_CH1_C   3
#define F_CORRUPTED_CH1_C_T   double
#define F_CORRUPTED_CH2_C   4
#define F_CORRUPTED_CH2_C_T   double
#define F_CORRUPTED_CH3_C   5
#define F_CORRUPTED_CH3_C_T   double
#define F_DEV_ADDR   6
#define F_DEV_ADDR_T   uint64_t
#define F_DST_ACK_C   7
#define F_DST_ACK_C_T   double
#define F_DST_NACK_C   8
#define F_DST_NACK_C_T   double
#define F_DST_SINGL_C   9
#define F_DST_SINGL_C_T   double
#define F_DST_SINGL_LM_T   10
#define F_DST_SINGL_LM_T_T   double
#define F_DST_TOTAL_C   11
#define F_DST_TOTAL_C_T   double
#define F_DST_TOTAL_LM_T   12
#define F_DST_TOTAL_LM_T_T   double
#define F_FAILED_HOP_NACK_C   13
#define F_FAILED_HOP_NACK_C_T   double
#define F_MULTICAST_C   14
#define F_MULTICAST_C_T   double
#define F_NET_MANAG_C   15
#define F_NET_MANAG_C_T   double
#define F_ROUTED_ACK_C   16
#define F_ROUTED_ACK_C_T   double
#define F_ROUTED_APP_C   17
#define F_ROUTED_APP_C_T   double
#define F_ROUTED_C   18
#define F_ROUTED_C_T   double
#define F_ROUTED_NACK_C   19
#define F_ROUTED_NACK_C_T   double
#define F_SINGLECAST_C   20
#define F_SINGLECAST_C_T   double
#define F_SRC_ACK_C   21
#define F_SRC_ACK_C_T   double
#define F_SRC_BROADCAST_C   22
#define F_SRC_BROADCAST_C_T   double
#define F_SRC_MULTICAST_C   23
#define F_SRC_MULTICAST_C_T   double
#define F_SRC_NACK_C   24
#define F_SRC_NACK_C_T   double
#define F_SRC_SINGL_C   25
#define F_SRC_SINGL_C_T   double
#define F_SRC_SINGL_LM_T   26
#define F_SRC_SINGL_LM_T_T   double
#define F_SRC_TOTAL_C   27
#define F_SRC_TOTAL_C_T   double
#define F_SRC_TOTAL_LM_T   28
#define F_SRC_TOTAL_LM_T_T   double
#define F_TIMESTAMP   29
#define F_TIMESTAMP_T   ur_time_t
#define F_TOTAL_OK_C   30
#define F_TOTAL_OK_C_T   double
#define F_CHANNEL   31
#define F_CHANNEL_T   uint8_t
#define F_FRAME   32
#define F_FRAME_T   char

extern uint16_t ur_last_id;
extern ur_static_field_specs_t UR_FIELD_SPECS_STATIC;
extern ur_field_specs_t ur_field_specs;

#endif

