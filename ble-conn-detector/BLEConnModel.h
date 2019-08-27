#ifndef BLE_CONN_MODEL_H
#define BLE_CONN_MODEL_H

#include <unirec/ur_time.h>

class BLEConnModel
{
public:
  virtual void receivedAdvAt(ur_time_t time) = 0;
  
  bool isReady(void) {
    return ready;
  }

protected:
  bool ready;
};

#endif
