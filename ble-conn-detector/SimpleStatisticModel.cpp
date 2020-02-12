#include <unirec/ur_time.h>

#include "SimpleStatisticModel.h"

SimpleStatisticModel::SimpleStatisticModel(void) {
	currThreshold = 0;
	initElements  = 10;
	lastSeen      = 0;
	silenceMidpoint = 0;
}

bool SimpleStatisticModel::isReady(void) {
	return initElements <= 0;
}

void SimpleStatisticModel::receivedAdvAt(ur_time_t time) {
	if (time == 0) // Invalid parameter
		return;

	if (lastSeen == 0) { // First occurence
		lastSeen = time;
		return;
	}

	uint64_t silenceDuration = ur_timediff(time, lastSeen);

	if (silenceMidpoint == 0) { // Second occurence -> initialise midpoint
		silenceMidpoint = silenceDuration;
		return;
	}

	uint64_t silenceDelta;

	// Calculate the delta od silenceDuration to the midpoint
	if (silenceDuration > silenceMidpoint)
		silenceDelta = silenceDuration - silenceMidpoint;
	else
		silenceDelta = silenceMidpoint - silenceDuration;


	if (initElements > 0) { // Model has not been initialised

		// Update midpoint to the mean of silence durations
		silenceMidpoint = (silenceMidpoint + silenceDuration) / 2;

		// Update threshold to the max of original threshold and current delta
		currThreshold = (silenceDelta > currThreshold) ? silenceDelta : currThreshold;

		if (--initElements <= 0) {
			lastSeen = time;
			throw SSMInitialised(silenceMidpoint, currThreshold);
		}

	} else { // Model properly initialised, do checks

		if (silenceDelta > 2*currThreshold) { // 2*threshold to eliminate smaller fluctuations

			lastSeen = time;
			throw ConnectionDetected(time, silenceDuration);

		} else { // Valid communication, update model

			// Update midpoint to the mean of silence durations
			silenceMidpoint = (silenceMidpoint + silenceDuration) / 2;

			// Update threshold to the max of original threshold and current delta
			currThreshold = (silenceDelta > currThreshold) ? silenceDelta : currThreshold;  

		}

	}

	lastSeen = time;
}

/* vim: set ts=3 sw=3 noet */
