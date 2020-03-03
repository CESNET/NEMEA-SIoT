/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
// Tables are indexed by field ID
#include "fields.h"

char *ur_field_names_static[] = {
   "TIMESTAMP",
   "DEV_ADDR",
   "INCIDENT_DEV_ADDR",
   "ALERT_CODE",
   "FCNT",
   "STATUS",
   "MS_TYPE",
   "CAPTION",
};
short ur_field_sizes_static[] = {
   8, /* TIMESTAMP */
   8, /* DEV_ADDR */
   8, /* INCIDENT_DEV_ADDR */
   4, /* ALERT_CODE */
   2, /* FCNT */
   1, /* STATUS */
   1, /* MS_TYPE */
   -1, /* CAPTION */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_TIME, /* TIMESTAMP */
   UR_TYPE_UINT64, /* DEV_ADDR */
   UR_TYPE_UINT64, /* INCIDENT_DEV_ADDR */
   UR_TYPE_UINT32, /* ALERT_CODE */
   UR_TYPE_UINT16, /* FCNT */
   UR_TYPE_UINT8, /* STATUS */
   UR_TYPE_UINT8, /* MS_TYPE */
   UR_TYPE_STRING, /* CAPTION */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 8};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 8, 8, 8, NULL, UR_UNINITIALIZED};
