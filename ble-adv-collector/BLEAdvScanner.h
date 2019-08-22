#ifndef BLE_ADV_SCANNER_H
#define BLE_ADV_SCANNER_H

class BLEAdvScanner
{
private:
	bdaddr_t bdaddr;
	uint16_t hciDevID;
	int      sock;

	void openHCISocket(void);
public:
	BLEAdvScanner(void);
	~BLEAdvScanner(void);

	const bdaddr_t* getBDAddr(void);
	const uint16_t getHCIDevice(void);

	void setPassiveMode(void);
	void start(void);
	void stop(void);
};

#endif
