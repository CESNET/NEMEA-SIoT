#ifndef BLE_ADV_SCANNER_H
#define BLE_ADV_SCANNER_H

#include <bluetooth/bluetooth.h>
#include <sys/time.h>

typedef struct {
	struct timeval timestamp;
	uint8_t  bdaddr_type; // Type of BDADDR: 0x00 = Public, 0x01 = Random
	bdaddr_t bdaddr;
	int8_t   rssi;
} adv_report;	

/* Events (Exceptions) */
struct BLEScannerError
{
public:
  const char* desc;

  BLEScannerError(const char *description) : desc(description) {}
};

/* Main scanner class */
class BLEAdvScanner
{
public:
	BLEAdvScanner(void);
	BLEAdvScanner(uint16_t hci_dev);
	~BLEAdvScanner(void);

	const bdaddr_t* getBDAddr(void);
	const uint16_t getHCIDevice(void);

	void setPassiveMode(void);
	void start(bool filter_dup);
	void stop(void);

	adv_report getAdvReport(void);
private:
	bdaddr_t bdaddr;
	uint16_t hciDevID;
	int      sock;
	bool     running;

	void openHCISocket(void);
};

#endif
