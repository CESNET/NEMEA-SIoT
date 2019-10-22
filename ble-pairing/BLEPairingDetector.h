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

#pragma once

#include <map>
#include <string>

#include <bluetooth/bluetooth.h>
#include <libtrap/trap.h>
#include <unirec/unirec.h>

#define EVENT_STATUS_SUCCESS 0x00

#define L2CAP_CID_SMP 0x0006

#define SMP_PAIRING_REQUEST  0x01
#define SMP_PAIRING_RESPONSE 0x02
#define SMP_PAIRING_FAILED   0x05

enum BLEPairingVersion {
	LEGACY_PAIRING = 0, //BLE 4.0
	SECURE_CONNECTIONS = 1, //BLE 4.2+
	UNKNOWN_VERSION = 255
};

enum BLEPairingMethod {
	JUST_WORKS = 0,
	PASSKEY_ENTRY = 1,
	OUT_OF_BAND = 2,
	NUMERIC_COMPARISON = 3,
	UNKNOWN_METHOD = 255
};

struct SMPPairingReqResp {
	uint8_t code;
	uint8_t ioCapability;
	uint8_t oobDataFlag;
	uint8_t authReq;
	uint8_t maxEncryptKeySize;
	uint8_t initiatorKeyDistribution;
	uint8_t responderKeyDistribution;
};
#define SMP_PARING_REQ_RESP_SIZE 7

/**
 * Process incoming Bluetooth HCI packets and detect BLE pairing process.
 */
class BLEPairingDetector {
public:
	/**
	 * Constructor
	 * @param[in] directoryPath Path where to load/store paired devices
	 * @param[in] verbose Level of verbosity
	 * @param[in] alert_template
	 * @param[in] alert_record
	 */
	BLEPairingDetector(
		std::string directoryPath,
		int verbose,
		ur_template_t *alert_template,
		void *alert_record);

	/**
	 * State of the connection.
	 */
	enum ConnectionState {
		STATE_CONNECTED,
		STATE_PAIRING_REQUEST,
		STATE_PAIRING_RESPONSE
	};

	/**
	 * Holds informations from pairing request/response needed
	 * to determine pairing method.
	 */
	struct PairingInfo {
		bool secureConnections = false;
		bool mitm = false;
		bool oob = false;
		uint8_t ioCapability;
	};

	/**
	 * Represents the state of the BLE connection along with informations
	 * needed for pairing alert.
	 */
	struct Connection {
		mac_addr_t address;
		ConnectionState state = STATE_CONNECTED;
		uint8_t pairingVersion = UNKNOWN_VERSION;
		uint8_t pairingMethod = UNKNOWN_METHOD;
		PairingInfo requestInfo;
	};

	/**
	 * @brief Based on the packet type call
	 * @param[in] hciDevMac Mac address of hci device from which the packet is
	 * @param[in] timestamp Timestamp of the packet
	 * @param[in] packetType Type of packet
	 * @param[in] packetSize Size of packet
	 * @param[in] packet Packet data
	 */
	void processPacket(
		mac_addr_t hciDevMac,
		ur_time_t timestamp,
		uint8_t packetType,
		uint16_t packetSize,
		uint8_t *packet);

private:
	/**
	 * @brief Process event type packets.
	 *  * EVT_LE_CONN_COMPLETE - store new BLE connection with state connected
	 *  * EVT_DISCONNECT_COMPLETE - free stored BLE connection and if there is
	 *      unfinished pairing process generate alert
	 *  * EVT_ENCRYPT_CHANGE - if there is finished pairing process
	 *      on the connection generate alert
	 * @param[in] hciDevMac Mac address of hci device from which the packet is
	 * @param[in] timestamp Timestamp of the packet
	 * @param[in] packet Packet data
	 * @param[in] packetSize Size of packet
	 */
	void processEvent(
		mac_addr_t hciDevMac,
		ur_time_t timestamp,
		uint8_t *packet,
		uint16_t packetSize);

	/**
	 * @brief Parse type of ACL Data packets and call processSMP for SMP packets.
	 * @param[in] hciDevMac Mac address of hci device from which the packet is
	 * @param[in] timestamp Timestamp of the packet
	 * @param[in] packet Packet data
	 * @param[in] packetSize Size of packet
	 */
	void processACLData(
		mac_addr_t hciDevMac,
		ur_time_t timestamp,
		uint8_t *packet,
		uint16_t packetSize);

	/**
	 * @brief Process 3 types of SMP packets.
	 *  * SMP_PAIRING_REQUEST - store pairing informations of the initiator
	 *      and connection goes to state pairing request
	 *  * SMP_PAIRING_RESPONSE - based on the saved pairing informations
	 *      of the initiator and informations of the responder determine
	 *      pairing version and method and connection goes to state pairing resp
	 *  * SMP_PAIRING_FAILED - generate alert
	 * @param[in] hciDevMac Mac address of hci device from which the packet is
	 * @param[in] timestamp Timestamp of the packet
	 * @param[in] packet Packet data
	 * @param[in] packetSize Size of packet
	 * @param[in] connectionHandle Identification of the connection
	 */
	void processSMP(
		mac_addr_t hciDevMac,
		ur_time_t timestamp,
		uint8_t *packet,
		uint16_t packetSize,
		uint16_t connectionHandle);

	/**
	 * @brief Extract needed values from SMP Pairing Request/Response packet.
	 * @param[in] data SMP Pairing Request/Response packet
	 * @return Needed pairing informations
	 */
	PairingInfo extractPairingInfo(SMPPairingReqResp *data);

	/**
	 * @brief Determine pairing method based on the pairing request/response.
	 * @param[in] secureConnections 1 = Secure Connections, 0 = Legacy Pairing
	 * @param[in] reqInfo
	 * @param[in] respInfo
	 * @return pairing method
	 */
	BLEPairingMethod findPairingMethod(
		bool secureConnections,
		const PairingInfo &reqInfo,
		const PairingInfo &respInfo);

	/**
	 * @brief Generate pairing alert (also determine if it is repeated
	 * pairing and persist paired device)
	 * @param[in] hciDevMac Mac address of hci device
	 * @param[in] timestamp Timestamp of the pairing
	 * @param[in] connection
	 * @param[in] success
	 */
	void generatePairingAlert(
		mac_addr_t hciDevMac,
		ur_time_t timestamp,
		const Connection &connection,
		bool success);

	bool isRepeatedPairing(const char *hciDevMac, const char *deviceMac);
	void persistPairing(const char *hciDevMac, const char *deviceMac);

private:
	std::string m_directoryPath;
	int m_verbose;
	ur_template_t *m_alert_template;
	void *m_alert_record;
	std::map<uint16_t , Connection> m_connectionMap; //Holds active BLE connections
};

