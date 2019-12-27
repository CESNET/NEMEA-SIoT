/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
// Tables are indexed by field ID
#include "fields.h"

char *ur_field_names_static[] = {
   "AIR_TIME",
   "DEV_ADDR",
   "INCIDENT_DEV_ADDR",
   "TIMESTAMP",
   "ALERT_CODE",
   "BAD_WIDTH",
   "CODE_RATE",
   "SF",
   "SIZE",
   "CAPTION",
   "PHY_PAYLOAD",
};
short ur_field_sizes_static[] = {
   8, /* AIR_TIME */
   8, /* DEV_ADDR */
   8, /* INCIDENT_DEV_ADDR */
   8, /* TIMESTAMP */
   4, /* ALERT_CODE */
   4, /* BAD_WIDTH */
   4, /* CODE_RATE */
   4, /* SF */
   4, /* SIZE */
   -1, /* CAPTION */
   -1, /* PHY_PAYLOAD */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_UINT64, /* AIR_TIME */
   UR_TYPE_UINT64, /* DEV_ADDR */
   UR_TYPE_UINT64, /* INCIDENT_DEV_ADDR */
   UR_TYPE_TIME, /* TIMESTAMP */
   UR_TYPE_UINT32, /* ALERT_CODE */
   UR_TYPE_UINT32, /* BAD_WIDTH */
   UR_TYPE_UINT32, /* CODE_RATE */
   UR_TYPE_UINT32, /* SF */
   UR_TYPE_UINT32, /* SIZE */
   UR_TYPE_STRING, /* CAPTION */
   UR_TYPE_STRING, /* PHY_PAYLOAD */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 11};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 11, 11, 11, NULL, UR_UNINITIALIZED};
