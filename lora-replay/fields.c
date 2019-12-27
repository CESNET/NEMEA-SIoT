/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
// Tables are indexed by field ID
#include "fields.h"

char *ur_field_names_static[] = {
   "DEV_ADDR",
   "INCIDENT_DEV_ADDR",
   "TIMESTAMP",
   "ALERT_CODE",
   "CAPTION",
   "FCNT",
   "PHY_PAYLOAD",
};
short ur_field_sizes_static[] = {
   8, /* DEV_ADDR */
   8, /* INCIDENT_DEV_ADDR */
   8, /* TIMESTAMP */
   4, /* ALERT_CODE */
   -1, /* CAPTION */
   -1, /* FCNT */
   -1, /* PHY_PAYLOAD */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_UINT64, /* DEV_ADDR */
   UR_TYPE_UINT64, /* INCIDENT_DEV_ADDR */
   UR_TYPE_TIME, /* TIMESTAMP */
   UR_TYPE_UINT32, /* ALERT_CODE */
   UR_TYPE_STRING, /* CAPTION */
   UR_TYPE_STRING, /* FCNT */
   UR_TYPE_STRING, /* PHY_PAYLOAD */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 7};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 7, 7, 7, NULL, UR_UNINITIALIZED};
