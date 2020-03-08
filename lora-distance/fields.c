/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
// Tables are indexed by field ID
#include "fields.h"

char *ur_field_names_static[] = {
   "BASE_RSSI",
   "DEV_ADDR",
   "INCIDENT_DEV_ADDR",
   "RSSI",
   "TIMESTAMP",
   "VARIANCE",
   "ALERT_CODE",
   "STATUS",
   "CAPTION",
};
short ur_field_sizes_static[] = {
   8, /* BASE_RSSI */
   8, /* DEV_ADDR */
   8, /* INCIDENT_DEV_ADDR */
   8, /* RSSI */
   8, /* TIMESTAMP */
   8, /* VARIANCE */
   4, /* ALERT_CODE */
   1, /* STATUS */
   -1, /* CAPTION */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_DOUBLE, /* BASE_RSSI */
   UR_TYPE_UINT64, /* DEV_ADDR */
   UR_TYPE_UINT64, /* INCIDENT_DEV_ADDR */
   UR_TYPE_DOUBLE, /* RSSI */
   UR_TYPE_TIME, /* TIMESTAMP */
   UR_TYPE_DOUBLE, /* VARIANCE */
   UR_TYPE_UINT32, /* ALERT_CODE */
   UR_TYPE_UINT8, /* STATUS */
   UR_TYPE_STRING, /* CAPTION */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 9};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 9, 9, 9, NULL, UR_UNINITIALIZED};
