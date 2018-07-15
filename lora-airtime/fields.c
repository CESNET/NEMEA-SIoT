/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
// Tables are indexed by field ID
#include "fields.h"

char *ur_field_names_static[] = {
   "AIR_TIME",
   "TIMESTAMP",
   "BAD_WIDTH",
   "CODE_RATE",
   "SF",
   "SIZE",
   "ENABLE",
   "APP_EUI",
   "APP_NONCE",
   "APP_SKEY",
   "DEV_ADDR",
   "DEV_EUI",
   "DEV_NONCE",
   "FCNT",
   "FCTRL",
   "FHDR",
   "F_OPTS",
   "F_PORT",
   "MAC_PAYLOAD",
   "MHDR",
   "MIC",
   "NET_ID",
   "NWK_SKEY",
   "PHY_PAYLOAD",
};
short ur_field_sizes_static[] = {
   8, /* AIR_TIME */
   8, /* TIMESTAMP */
   4, /* BAD_WIDTH */
   4, /* CODE_RATE */
   4, /* SF */
   4, /* SIZE */
   1, /* ENABLE */
   -1, /* APP_EUI */
   -1, /* APP_NONCE */
   -1, /* APP_SKEY */
   -1, /* DEV_ADDR */
   -1, /* DEV_EUI */
   -1, /* DEV_NONCE */
   -1, /* FCNT */
   -1, /* FCTRL */
   -1, /* FHDR */
   -1, /* F_OPTS */
   -1, /* F_PORT */
   -1, /* MAC_PAYLOAD */
   -1, /* MHDR */
   -1, /* MIC */
   -1, /* NET_ID */
   -1, /* NWK_SKEY */
   -1, /* PHY_PAYLOAD */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_UINT64, /* AIR_TIME */
   UR_TYPE_UINT64, /* TIMESTAMP */
   UR_TYPE_UINT32, /* BAD_WIDTH */
   UR_TYPE_UINT32, /* CODE_RATE */
   UR_TYPE_UINT32, /* SF */
   UR_TYPE_UINT32, /* SIZE */
   UR_TYPE_UINT8, /* ENABLE */
   UR_TYPE_STRING, /* APP_EUI */
   UR_TYPE_STRING, /* APP_NONCE */
   UR_TYPE_STRING, /* APP_SKEY */
   UR_TYPE_STRING, /* DEV_ADDR */
   UR_TYPE_STRING, /* DEV_EUI */
   UR_TYPE_STRING, /* DEV_NONCE */
   UR_TYPE_STRING, /* FCNT */
   UR_TYPE_STRING, /* FCTRL */
   UR_TYPE_STRING, /* FHDR */
   UR_TYPE_STRING, /* F_OPTS */
   UR_TYPE_STRING, /* F_PORT */
   UR_TYPE_STRING, /* MAC_PAYLOAD */
   UR_TYPE_STRING, /* MHDR */
   UR_TYPE_STRING, /* MIC */
   UR_TYPE_STRING, /* NET_ID */
   UR_TYPE_STRING, /* NWK_SKEY */
   UR_TYPE_STRING, /* PHY_PAYLOAD */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 24};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 24, 24, 24, NULL, UR_UNINITIALIZED};
