/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
// Tables are indexed by field ID
#include "fields.h"

char *ur_field_names_static[] = {
   "EVENT_TYPE",
   "HOME_ID",
   "INCIDENT_DEV_ADDR",
   "NODE_ID",
   "TIME",
   "TIMESTAMP",
   "ALERT_CODE",
   "CHANNEL",
   "CAPTION",
   "FRAME",
};
short ur_field_sizes_static[] = {
   8, /* EVENT_TYPE */
   8, /* HOME_ID */
   8, /* INCIDENT_DEV_ADDR */
   8, /* NODE_ID */
   8, /* TIME */
   8, /* TIMESTAMP */
   4, /* ALERT_CODE */
   1, /* CHANNEL */
   -1, /* CAPTION */
   -1, /* FRAME */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_DOUBLE, /* EVENT_TYPE */
   UR_TYPE_DOUBLE, /* HOME_ID */
   UR_TYPE_UINT64, /* INCIDENT_DEV_ADDR */
   UR_TYPE_DOUBLE, /* NODE_ID */
   UR_TYPE_TIME, /* TIME */
   UR_TYPE_TIME, /* TIMESTAMP */
   UR_TYPE_UINT32, /* ALERT_CODE */
   UR_TYPE_UINT8, /* CHANNEL */
   UR_TYPE_STRING, /* CAPTION */
   UR_TYPE_BYTES, /* FRAME */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 10};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 10, 10, 10, NULL, UR_UNINITIALIZED};
