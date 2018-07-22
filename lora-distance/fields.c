/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
// Tables are indexed by field ID
#include "fields.h"

char *ur_field_names_static[] = {
   "BASE_RSSI",
   "RSSI",
   "TIMESTAMP",
   "VARIANCE",
   "DEV_ADDR",
   "PHY_PAYLOAD",
};
short ur_field_sizes_static[] = {
   8, /* BASE_RSSI */
   8, /* RSSI */
   8, /* TIMESTAMP */
   8, /* VARIANCE */
   -1, /* DEV_ADDR */
   -1, /* PHY_PAYLOAD */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_DOUBLE, /* BASE_RSSI */
   UR_TYPE_DOUBLE, /* RSSI */
   UR_TYPE_UINT64, /* TIMESTAMP */
   UR_TYPE_DOUBLE, /* VARIANCE */
   UR_TYPE_STRING, /* DEV_ADDR */
   UR_TYPE_STRING, /* PHY_PAYLOAD */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 6};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 6, 6, 6, NULL, UR_UNINITIALIZED};
