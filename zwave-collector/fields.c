/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
// Tables are indexed by field ID
#include "fields.h"

char *ur_field_names_static[] = {
   "DEV_ADDR",
   "TIMESTAMP",
   "HOME_ID",
   "ACK_REQ",
   "CHANNEL",
   "DST_HOP",
   "DST_ID",
   "FAILED_HOP",
   "ROUTED",
   "SEQ_NUM",
   "SIZE",
   "SRC_HOP",
   "CMD_CLASS_STR",
   "CMD_STR",
   "FRAME",
   "HOPS",
   "PAYLOAD",
   "ROUTED_TYPE",
   "TYPE",
};
short ur_field_sizes_static[] = {
   8, /* DEV_ADDR */
   8, /* TIMESTAMP */
   4, /* HOME_ID */
   1, /* ACK_REQ */
   1, /* CHANNEL */
   1, /* DST_HOP */
   1, /* DST_ID */
   1, /* FAILED_HOP */
   1, /* ROUTED */
   1, /* SEQ_NUM */
   1, /* SIZE */
   1, /* SRC_HOP */
   -1, /* CMD_CLASS_STR */
   -1, /* CMD_STR */
   -1, /* FRAME */
   -1, /* HOPS */
   -1, /* PAYLOAD */
   -1, /* ROUTED_TYPE */
   -1, /* TYPE */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_UINT64, /* DEV_ADDR */
   UR_TYPE_TIME, /* TIMESTAMP */
   UR_TYPE_UINT32, /* HOME_ID */
   UR_TYPE_UINT8, /* ACK_REQ */
   UR_TYPE_UINT8, /* CHANNEL */
   UR_TYPE_UINT8, /* DST_HOP */
   UR_TYPE_UINT8, /* DST_ID */
   UR_TYPE_UINT8, /* FAILED_HOP */
   UR_TYPE_UINT8, /* ROUTED */
   UR_TYPE_UINT8, /* SEQ_NUM */
   UR_TYPE_UINT8, /* SIZE */
   UR_TYPE_UINT8, /* SRC_HOP */
   UR_TYPE_STRING, /* CMD_CLASS_STR */
   UR_TYPE_STRING, /* CMD_STR */
   UR_TYPE_BYTES, /* FRAME */
   UR_TYPE_BYTES, /* HOPS */
   UR_TYPE_BYTES, /* PAYLOAD */
   UR_TYPE_STRING, /* ROUTED_TYPE */
   UR_TYPE_STRING, /* TYPE */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 19};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 19, 19, 19, NULL, UR_UNINITIALIZED};
