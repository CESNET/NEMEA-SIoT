#ifndef BLE_SIMPLE_STAT_MODEL_H
#define BLE_SIMPLE_STAT_MODEL_H

#include <unirec/ur_time.h>
#include <vector>

#include "BLEConnModel.h"

/* Events (Exceptions) */
struct SSMInitialised : public ModelInitialised
{
	const uint64_t midpoint;
	const uint64_t threshold;

	SSMInitialised(uint64_t midpoint, uint64_t threshold)
		: midpoint(midpoint)
		, threshold(threshold)
		{}
};


/* Main model class */
class SimpleStatisticModel: public BLEConnModel
{
public:
	SimpleStatisticModel(void);
	void receivedAdvAt(ur_time_t time);
	bool isReady(void);
protected:
	uint64_t  currThreshold;  // current silenceDuration value threshold
	uint16_t  initElements;   // number of elements needed for model initialization
	ur_time_t lastSeen;       // timestamp of the last advertisement
	uint64_t  silenceMidpoint;// mean value of the silence duration between two adverisements
};

#endif

/* vim: set ts=3 sw=3 noet */
