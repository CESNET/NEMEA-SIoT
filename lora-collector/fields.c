/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
// Tables are indexed by field ID
#include "fields.h"

char *ur_field_names_static[] = {
   "RSSI",
   "TIMESTAMP",
   "BAD_WIDTH",
   "CODE_RATE",
   "SF",
   "SIZE",
   "PHY_PAYLOAD",
};
short ur_field_sizes_static[] = {
   8, /* RSSI */
   8, /* TIMESTAMP */
   4, /* BAD_WIDTH */
   4, /* CODE_RATE */
   4, /* SF */
   4, /* SIZE */
   -1, /* PHY_PAYLOAD */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_DOUBLE, /* RSSI */
   UR_TYPE_UINT64, /* TIMESTAMP */
   UR_TYPE_UINT32, /* BAD_WIDTH */
   UR_TYPE_UINT32, /* CODE_RATE */
   UR_TYPE_UINT32, /* SF */
   UR_TYPE_UINT32, /* SIZE */
   UR_TYPE_STRING, /* PHY_PAYLOAD */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 7};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 7, 7, 7, NULL, UR_UNINITIALIZED};
