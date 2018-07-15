/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
// Tables are indexed by field ID
#include "fields.h"

char *ur_field_names_static[] = {
   "TIMESTAMP",
   "DEVICE_MAC",
   "HCI_DEV_MAC",
   "DATA_DIRECTION",
   "METHOD",
   "PACKET_TYPE",
   "REPEATED",
   "SUCCESS",
   "VERSION",
   "PACKET",
};
short ur_field_sizes_static[] = {
   8, /* TIMESTAMP */
   6, /* DEVICE_MAC */
   6, /* HCI_DEV_MAC */
   1, /* DATA_DIRECTION */
   1, /* METHOD */
   1, /* PACKET_TYPE */
   1, /* REPEATED */
   1, /* SUCCESS */
   1, /* VERSION */
   -1, /* PACKET */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_TIME, /* TIMESTAMP */
   UR_TYPE_MAC, /* DEVICE_MAC */
   UR_TYPE_MAC, /* HCI_DEV_MAC */
   UR_TYPE_UINT8, /* DATA_DIRECTION */
   UR_TYPE_UINT8, /* METHOD */
   UR_TYPE_UINT8, /* PACKET_TYPE */
   UR_TYPE_UINT8, /* REPEATED */
   UR_TYPE_UINT8, /* SUCCESS */
   UR_TYPE_UINT8, /* VERSION */
   UR_TYPE_BYTES, /* PACKET */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 10};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 10, 10, 10, NULL, UR_UNINITIALIZED};
