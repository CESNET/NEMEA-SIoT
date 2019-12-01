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
  if (time == 0)  // invalid parameter
    return;

  if (lastSeen != 0) {  // Model has been initialised
    uint64_t silenceDuration = ur_timediff(time, lastSeen);
    uint64_t silenceDelta;

    if (silenceMidpoint == 0) // Initialise midpoint
      silenceMidpoint = silenceDuration;

    if (silenceDuration > silenceMidpoint)
      silenceDelta = silenceDuration - silenceMidpoint;
    else
      silenceDelta = silenceMidpoint - silenceDuration;

    // if (silenceDelta <= currThreshold)
    currThreshold = (silenceDelta > currThreshold) ? silenceDelta : currThreshold;
  }

  if (initElements > 0) {
    initElements--;

    if (initElements <= 0)
      throw SSMInitialised(silenceMidpoint, currThreshold);
  }

  lastSeen = time;
}
