/**
 * @file BLEPairingDetector.h
 * @brief Analyze received Bluetooth packets from HCI and detect BLE pairing.
 * @author Jozef Halaj <xhalaj03@stud.fit.vutbr.cz>
 * @date 2018
 */

/*
 * Copyright (C) 2018 CESNET
 *
 * LICENSE TERMS
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * This software is provided ``as is'', and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
*/

#include <csignal>
#include <fstream>
#include <iostream>
#include <getopt.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/l2cap.h>

#include "BLEPairingDetector.h"
#include "fields.h"

using namespace std;

trap_module_info_t *module_info = NULL;

#define MODULE_BASIC_INFO(BASIC) \
	BASIC("ble-pairing-detector", \
		"Analyze received packets from bluetooth-hci-collector and detect BLE pairing.", 1, 1)
#define MODULE_PARAMS(PARAM) \
	PARAM('d', "dir", "Directory to store/load paired devices.", required_argument, "string") \
    PARAM('I', "ignore-in-eof", "Do not terminate on incomming termination message.", no_argument, "none")

static int g_stop = 0;

TRAP_DEFAULT_SIGNAL_HANDLER(g_stop = 1)

int main(int argc, char *argv[])
{
	int exit_value = 0;
	ur_template_t *in_template = NULL;
	ur_template_t *alert_template = NULL;
	void *alert_record = NULL;
	int opt;
	int verbose = 0;
	string directory = ".";
    int ignore_eof = 0; // Ignore EOF input parameter flag

	INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
	TRAP_DEFAULT_INITIALIZATION(argc, argv, *module_info);

	TRAP_REGISTER_DEFAULT_SIGNAL_HANDLER();

	while ((opt = getopt_long(argc, argv, module_getopt_string, long_options, NULL)) != -1) {
		switch (opt) {
			case 'd':
				directory = optarg;
				break;
            case 'I':
                ignore_eof = 1;
                break;
			default:
				cerr << "Invalid argument." << endl;
				TRAP_DEFAULT_FINALIZATION();
				FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
				return exit_value;
		}
	}

	if (trap_ifcctl(TRAPIFC_INPUT, 0, TRAPCTL_SETTIMEOUT, TRAP_WAIT) != TRAP_E_OK) {
		cerr << "Error: could not set input interface timeout." << endl;
		exit_value = -1;
		goto cleanup;
	}

	in_template = ur_create_input_template(0, "HCI_DEV_MAC, TIMESTAMP, DATA_DIRECTION, PACKET_TYPE, PACKET", NULL);
	if (in_template == NULL) {
		cerr << "Error: input template could not be created." << endl;
		exit_value = -1;
		goto cleanup;
	}

	alert_template = ur_create_output_template(0, "HCI_DEV_MAC, DEVICE_MAC, TIMESTAMP, SUCCESS, REPEATED, VERSION, METHOD", NULL);
	if (alert_template == NULL) {
		cerr << "Error: alert template could not be created." << endl;
		exit_value = -1;
		goto cleanup;
	}

	alert_record = ur_create_record(alert_template, 0);
	if (alert_record == NULL) {
		cerr << "Error: Memory allocation problem (output record).";
		exit_value = -1;
		goto cleanup;
	}

	verbose = trap_get_verbose_level();

	{
		BLEPairingDetector detector(directory, verbose, alert_template, alert_record);

		while(!g_stop) {

			const void *in_record;
			uint16_t in_record_size;

			int ret = TRAP_RECEIVE(0, in_record, in_record_size, in_template);
			TRAP_DEFAULT_RECV_ERROR_HANDLING(ret, continue, break);

            // EOF close this module 
            if ( in_record_size <= 1 ){
                char dummy[1] = {0};
                trap_send(0, dummy, 1); 
                trap_send_flush(0);
                // if ignore_eof option is used -> forward eof message but keep this module running
                if ( !ignore_eof ){
                    goto cleanup;
                } 
            }
            


			detector.processPacket(
				ur_get_ptr(in_template, in_record, F_HCI_DEV_MAC),
				ur_get_ptr(in_template, in_record, F_TIMESTAMP),
				ur_get_ptr(in_template, in_record, F_PACKET_TYPE),
				ur_get_var_len(in_template, in_record, F_PACKET),
				(uint8_t *) ur_get_ptr(in_template, in_record, F_PACKET)
			);
		}
	}

cleanup:
	ur_free_template(in_template);

	ur_free_record(alert_record);
	ur_free_template(alert_template);

	TRAP_DEFAULT_FINALIZATION();
	FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)

	return exit_value;
}

BLEPairingDetector::BLEPairingDetector(
		string directoryPath,
		int verbose,
		ur_template_t *alert_template,
		void *alert_record):
	m_directoryPath(directoryPath),
	m_verbose(verbose),
	m_alert_template(alert_template),
	m_alert_record(alert_record)
{
}

void BLEPairingDetector::processPacket(
	mac_addr_t *hciDevMac,
	ur_time_t *timestamp,
	uint8_t *packetType,
	uint16_t packetSize,
	uint8_t *packet)
{
	switch (*packetType) {
		case HCI_EVENT_PKT:
			processEvent(hciDevMac, timestamp, packet, packetSize);
			break;
		case HCI_ACLDATA_PKT:
			processACLData(hciDevMac, timestamp, packet, packetSize);
			break;
		case HCI_COMMAND_PKT:
		case HCI_SCODATA_PKT:
			break;
		default:
			cerr << "error: bad packet type" << endl;
	}
}

void BLEPairingDetector::processEvent(
	mac_addr_t *hciDevMac,
	ur_time_t *timestamp,
	uint8_t *packet,
	uint16_t packetSize)
{
	if (packetSize < 2) { //event header
		cerr << "error: invalid size of packet" << endl;
		return;
	}

	const hci_event_hdr *hdr = (hci_event_hdr *) packet;
	if (packetSize - 2 != hdr->plen) {
		cerr << "error: invalid size of packet" << endl;
		return;
	}

	if (hdr->evt == EVT_LE_META_EVENT) {
		evt_le_meta_event *event = (evt_le_meta_event *) (packet + 2);
		if (event->subevent == EVT_LE_CONN_COMPLETE) {
			if (hdr->plen != EVT_LE_META_EVENT_SIZE + EVT_LE_CONN_COMPLETE_SIZE) {
				cerr << "error: invalid size of packet" << endl;
				return;
			}

			evt_le_connection_complete *eventLE = (evt_le_connection_complete *) event->data;
			if (eventLE->status != EVENT_STATUS_SUCCESS)
				return;

			uint16_t handle = htobl(eventLE->handle);

			Connection connection;
			baswap(&connection.address, &eventLE->peer_bdaddr);
			connection.state = STATE_CONNECTED;

			m_connectionMap.insert(make_pair(handle, connection));
		}
	}
	else if (hdr->evt == EVT_DISCONN_COMPLETE) {
		if (hdr->plen != EVT_DISCONN_COMPLETE_SIZE) {
			cerr << "error: invalid size of packet" << endl;
			return;
		}

		evt_disconn_complete *event = (evt_disconn_complete *) (packet + 2);

		uint16_t handle = htobl(event->handle);

		auto it = m_connectionMap.find(handle);
		if (it == m_connectionMap.end())
			return;

		if (it->second.state != STATE_CONNECTED)
			generatePairingAlert(hciDevMac, timestamp, it->second, false);

		m_connectionMap.erase(it);
	}
	else if (hdr->evt == EVT_ENCRYPT_CHANGE) {
		if (hdr->plen != EVT_ENCRYPT_CHANGE_SIZE) {
			cerr << "error: invalid size of packet" << endl;
			return;
		}

		evt_encrypt_change *event = (evt_encrypt_change *) (packet + 2);

		uint16_t handle = htobl(event->handle);

		auto it = m_connectionMap.find(handle);
		if (it == m_connectionMap.end())
			return;

		if (it->second.state == STATE_PAIRING_RESPONSE) {
			it->second.state = STATE_CONNECTED;

			if (event->status == EVENT_STATUS_SUCCESS)
				generatePairingAlert(hciDevMac, timestamp, it->second, true);
			else
				generatePairingAlert(hciDevMac, timestamp, it->second, false);
		}
	}
}

void BLEPairingDetector::processACLData(
	mac_addr_t *hciDevMac,
	ur_time_t *timestamp,
	uint8_t *packet,
	uint16_t packetSize)
{
	if (packetSize < 8) { //acl header + l2cap header
		cerr << "error: invalid size of packet" << endl;
		return;
	}

	hci_acl_hdr *hdr = (hci_acl_hdr *) packet;
	if (packetSize - 4 != hdr->dlen) {
		cerr << "error: invalid size of packet" << endl;
		return;
	}

	uint16_t handle = htobl(hdr->handle) & 0x0FFF; // first 4 bits are ACL flags

	l2cap_hdr *hdrL2CAP = (l2cap_hdr *) (packet + 4);
	if (htobl(hdrL2CAP->cid) != L2CAP_CID_SMP)
		return;

	processSMP(hciDevMac, timestamp, packet + 8, hdrL2CAP->len, handle);
}

void BLEPairingDetector::processSMP(
	mac_addr_t *hciDevMac,
	ur_time_t *timestamp,
	uint8_t *packet,
	uint16_t packetSize,
	uint16_t connectionHandle)
{
	if (packetSize < 1) { //atleast smp opcode to read
		cerr << "error: invalid size of packet" << endl;
		return;
	}

	uint8_t opcode = *packet;

	if (opcode == SMP_PAIRING_REQUEST) {
		if (packetSize != SMP_PARING_REQ_RESP_SIZE) {
			cerr << "error: invalid size of packet" << endl;
			return;
		}

		auto it = m_connectionMap.find(connectionHandle);
		if (it == m_connectionMap.end())
			return;

		it->second.state = STATE_PAIRING_REQUEST;

		SMPPairingReqResp *pairingReq = (SMPPairingReqResp *) packet;

		it->second.requestInfo = extractPairingInfo(pairingReq);

	}
	else if (opcode == SMP_PAIRING_RESPONSE) {
		if (packetSize != SMP_PARING_REQ_RESP_SIZE) {
			cerr << "error: invalid size of packet" << endl;
			return;
		}

		auto it = m_connectionMap.find(connectionHandle);
		if (it == m_connectionMap.end())
			return;

		it->second.state = STATE_PAIRING_RESPONSE;

		SMPPairingReqResp *pairingRes = (SMPPairingReqResp *) packet;

		PairingInfo respInfo = extractPairingInfo(pairingRes);
		const PairingInfo &reqInfo = it->second.requestInfo;

		bool secureConnections = reqInfo.secureConnections && respInfo.secureConnections;

		it->second.pairingVersion = secureConnections;
		it->second.pairingMethod = findPairingMethod(secureConnections, reqInfo, respInfo);
	}
	else if (opcode == SMP_PAIRING_FAILED) {
		auto it = m_connectionMap.find(connectionHandle);
		if (it == m_connectionMap.end())
			return;

		it->second.state = STATE_CONNECTED;

		generatePairingAlert(hciDevMac, timestamp, it->second, false);
	}
}

BLEPairingDetector::PairingInfo BLEPairingDetector::extractPairingInfo(SMPPairingReqResp *data)
{
	PairingInfo info;

	info.secureConnections = (data->authReq & 0b00001000) >> 3; //SC bit from AuthReq
	info.mitm = (data->authReq & 0b00000100) >> 2; //MITM bit from AuthReq

	if (data->oobDataFlag == 1)
		info.oob = true;

	info.ioCapability = data->ioCapability;

	return info;
}

BLEPairingMethod BLEPairingDetector::findPairingMethod(
	bool secureConnections,
	const PairingInfo &reqInfo,
	const PairingInfo &respInfo)
{
	//Bluetooth Specification v5.0 page 2312

	static const BLEPairingMethod legacyPairingTable[5][5] = {
		{JUST_WORKS, JUST_WORKS, PASSKEY_ENTRY, JUST_WORKS, PASSKEY_ENTRY},
		{JUST_WORKS, JUST_WORKS, PASSKEY_ENTRY, JUST_WORKS, PASSKEY_ENTRY},
		{PASSKEY_ENTRY, PASSKEY_ENTRY, PASSKEY_ENTRY, JUST_WORKS, PASSKEY_ENTRY},
		{JUST_WORKS, JUST_WORKS, JUST_WORKS, JUST_WORKS, JUST_WORKS},
		{PASSKEY_ENTRY, PASSKEY_ENTRY, PASSKEY_ENTRY, JUST_WORKS, PASSKEY_ENTRY}
	};

	static const BLEPairingMethod secureConnectionsTable[5][5] = {
		{JUST_WORKS, JUST_WORKS, PASSKEY_ENTRY, JUST_WORKS, PASSKEY_ENTRY},
		{JUST_WORKS, NUMERIC_COMPARISON, PASSKEY_ENTRY, JUST_WORKS, NUMERIC_COMPARISON},
		{PASSKEY_ENTRY, PASSKEY_ENTRY, PASSKEY_ENTRY, JUST_WORKS, PASSKEY_ENTRY},
		{JUST_WORKS, JUST_WORKS, JUST_WORKS, JUST_WORKS, JUST_WORKS},
		{PASSKEY_ENTRY, NUMERIC_COMPARISON, PASSKEY_ENTRY, JUST_WORKS, NUMERIC_COMPARISON}
	};

	if (!secureConnections) { //legacy pairing
		if (reqInfo.oob && respInfo.oob) {
			return OUT_OF_BAND;
		}
		else {
			if (!reqInfo.mitm && !respInfo.mitm) {
				return JUST_WORKS;
			}
			else {
				if (reqInfo.ioCapability > 4 || respInfo.ioCapability > 4)
					return UNKNOWN_METHOD;
				else
					return legacyPairingTable[respInfo.ioCapability][reqInfo.ioCapability];
			}
		}
	}
	else { //secure connections
		if (!reqInfo.oob && !respInfo.oob) {
			if (!reqInfo.mitm && !respInfo.mitm) {
				return JUST_WORKS;
			}
			else {
				if (reqInfo.ioCapability > 4 || respInfo.ioCapability > 4)
					return UNKNOWN_METHOD;
				else
					return secureConnectionsTable[respInfo.ioCapability][reqInfo.ioCapability];
			}
		}
		else {
			return OUT_OF_BAND;
		}
	}
}

void BLEPairingDetector::generatePairingAlert(
	mac_addr_t *hciDevMac,
	ur_time_t *timestamp,
	const Connection &connection,
	bool success)
{
	char hciDevMacStr[17];
	mac_to_str(hciDevMac, hciDevMacStr);
	char *deviceMacStr = batostr(&connection.address);

	bool isRepeated = isRepeatedPairing(hciDevMacStr, deviceMacStr);

	if (!isRepeated && success)
		persistPairing(hciDevMacStr, deviceMacStr);

	if (m_verbose > 0) {
		cout << "pairing of device: " << deviceMacStr
			 << " on hci dev: " << hciDevMacStr
			 << ", timestamp: " << ur_time_get_sec(*timestamp) << "." << ur_time_get_msec(*timestamp)
			 << ", success: " << success
			 << ", repeated: " << isRepeated
			 << ", version: " << (int) connection.pairingVersion
			 << ", method: " << (int) connection.pairingMethod
			 << endl;
	}

	ur_set(m_alert_template, m_alert_record, F_HCI_DEV_MAC,*hciDevMac);
	ur_set(m_alert_template, m_alert_record, F_DEVICE_MAC, mac_from_bytes((uint8_t *) connection.address.b));
	ur_set(m_alert_template, m_alert_record, F_TIMESTAMP, *timestamp);
	ur_set(m_alert_template, m_alert_record, F_SUCCESS, success);
	ur_set(m_alert_template, m_alert_record, F_REPEATED, isRepeated);
	ur_set(m_alert_template, m_alert_record, F_VERSION, connection.pairingVersion);
	ur_set(m_alert_template, m_alert_record, F_METHOD, connection.pairingMethod);

	trap_send(0, m_alert_record, ur_rec_size(m_alert_template, m_alert_record));
}

bool BLEPairingDetector::isRepeatedPairing(const char *hciDevMac, const char *deviceMac)
{
	ifstream file(m_directoryPath + "/" + hciDevMac + "-" + deviceMac);

	return file.good();
}

void BLEPairingDetector::persistPairing(const char *hciDevMac, const char *deviceMac)
{
	ofstream file(m_directoryPath + "/" + hciDevMac + "-" + deviceMac);
	if (!file.is_open())
		cerr << "error: failed to persist pairing of " << deviceMac << " on " << hciDevMac << endl;
}
