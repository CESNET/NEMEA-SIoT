/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
// Tables are indexed by field ID
#include "fields.h"

char *ur_field_names_static[] = {
   "TIMESTAMP",
   "AIR_TIME",
   "DEV_ADDR",
   "INCIDENT_DEV_ADDR",
   "BAD_WIDTH",
   "SF",
   "CODE_RATE",
   "ALERT_CODE",
   "SIZE",
   "STATUS",
   "MS_TYPE",
   "CAPTION",
};
short ur_field_sizes_static[] = {
   8, /* TIMESTAMP */
   8, /* AIR_TIME */
   8, /* DEV_ADDR */
   8, /* INCIDENT_DEV_ADDR */
   4, /* BAD_WIDTH */
   4, /* SF */
   4, /* CODE_RATE */
   4, /* ALERT_CODE */
   2, /* SIZE */
   1, /* STATUS */
   1, /* MS_TYPE */
   -1, /* CAPTION */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_TIME, /* TIMESTAMP */
   UR_TYPE_DOUBLE, /* AIR_TIME */
   UR_TYPE_UINT64, /* DEV_ADDR */
   UR_TYPE_UINT64, /* INCIDENT_DEV_ADDR */
   UR_TYPE_UINT32, /* BAD_WIDTH */
   UR_TYPE_UINT32, /* SF */
   UR_TYPE_UINT32, /* CODE_RATE */
   UR_TYPE_UINT32, /* ALERT_CODE */
   UR_TYPE_UINT16, /* SIZE */
   UR_TYPE_UINT8, /* STATUS */
   UR_TYPE_UINT8, /* MS_TYPE */
   UR_TYPE_STRING, /* CAPTION */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 12};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 12, 12, 12, NULL, UR_UNINITIALIZED};
