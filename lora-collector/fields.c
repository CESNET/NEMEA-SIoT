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
   "RF_CHAIN",
   "SNR",
   "DEV_ADDR",
   "FRQ",
   "US_COUNT",
   "STATUS",
   "MOD",
   "FCNT",
   "MS_TYPE",
   "PHY_PAYLOAD",
   "APP_EUI",
   "DEV_EUI",
   "FOPTS",
   "FPORT",
   "DEV_NONCE",
   "FCTRL",
   "FHDR",
   "APP_NONCE",
   "MHDR",
   "MIC",
   "NET_ID",
};
short ur_field_sizes_static[] = {
   8, /* RSSI */
   8, /* TIMESTAMP */
   4, /* BAD_WIDTH */
   4, /* CODE_RATE */
   4, /* SF */
   2, /* SIZE */
   1, /* RF_CHAIN */
   8, /* SNR */
   8, /* DEV_ADDR */
   4, /* FRQ */
   4, /* US_COUNT */
   1, /* STATUS */
   1, /* MOD */
   2, /* FCNT */
   1, /* MS_TYPE */
   -1, /* PHY_PAYLOAD */
   -1, /* APP_EUI */
   -1, /* DEV_EUI */
   -1, /* FOPTS */
   -1, /* FPORT */
   -1, /* DEV_NONCE */
   -1, /* FCTRL */
   -1, /* FHDR */
   -1, /* APP_NONCE */
   -1, /* MHDR */
   -1, /* MIC */
   -1, /* NET_ID */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_DOUBLE, /* RSSI */
   UR_TYPE_TIME, /* TIMESTAMP */
   UR_TYPE_UINT32, /* BAD_WIDTH */
   UR_TYPE_UINT32, /* CODE_RATE */
   UR_TYPE_UINT32, /* SF */
   UR_TYPE_UINT16, /* SIZE */
   UR_TYPE_UINT8, /* RF_CHAIN */
   UR_TYPE_DOUBLE, /* SNR */
   UR_TYPE_UINT64, /* DEV_ADDR */
   UR_TYPE_UINT32, /* FRQ */
   UR_TYPE_UINT32, /* US_COUNT */
   UR_TYPE_UINT8, /* STATUS */
   UR_TYPE_UINT8, /* MOD */
   UR_TYPE_UINT16, /* FCNT */
   UR_TYPE_UINT8, /* MS_TYPE */
   UR_TYPE_STRING, /* PHY_PAYLOAD */
   UR_TYPE_STRING, /* APP_EUI */
   UR_TYPE_STRING, /* DEV_EUI */
   UR_TYPE_STRING, /* FOPTS */
   UR_TYPE_STRING, /* FPORT */
   UR_TYPE_STRING, /* DEV_NONCE */
   UR_TYPE_STRING, /* FCTRL */
   UR_TYPE_STRING, /* FHDR */
   UR_TYPE_STRING, /* APP_NONCE */
   UR_TYPE_STRING, /* MHDR */
   UR_TYPE_STRING, /* MIC */
   UR_TYPE_STRING, /* NET_ID */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 27};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 27, 27, 27, NULL, UR_UNINITIALIZED};
