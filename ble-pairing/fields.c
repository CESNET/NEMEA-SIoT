/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
// Tables are indexed by field ID
#include "fields.h"

char *ur_field_names_static[] = {
   "TIMESTAMP",
   "DEV_ADDR",
   "HCI_DEV_ADDR",
   "INCIDENT_DEV_ADDR",
   "ALERT_CODE",
   "SIZE",
   "DATA_DIRECTION",
   "METHOD",
   "PACKET_TYPE",
   "SUCCESS",
   "VERSION",
   "CAPTION",
   "PACKET",
};
short ur_field_sizes_static[] = {
   8, /* TIMESTAMP */
   6, /* DEV_ADDR */
   6, /* HCI_DEV_ADDR */
   6, /* INCIDENT_DEV_ADDR */
   4, /* ALERT_CODE */
   2, /* SIZE */
   1, /* DATA_DIRECTION */
   1, /* METHOD */
   1, /* PACKET_TYPE */
   1, /* SUCCESS */
   1, /* VERSION */
   -1, /* CAPTION */
   -1, /* PACKET */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_TIME, /* TIMESTAMP */
   UR_TYPE_MAC, /* DEV_ADDR */
   UR_TYPE_MAC, /* HCI_DEV_ADDR */
   UR_TYPE_MAC, /* INCIDENT_DEV_ADDR */
   UR_TYPE_UINT32, /* ALERT_CODE */
   UR_TYPE_UINT16, /* SIZE */
   UR_TYPE_UINT8, /* DATA_DIRECTION */
   UR_TYPE_UINT8, /* METHOD */
   UR_TYPE_UINT8, /* PACKET_TYPE */
   UR_TYPE_UINT8, /* SUCCESS */
   UR_TYPE_UINT8, /* VERSION */
   UR_TYPE_STRING, /* CAPTION */
   UR_TYPE_BYTES, /* PACKET */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 13};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 13, 13, 13, NULL, UR_UNINITIALIZED};
