#ifndef BLE_CONN_MODEL_H
#define BLE_CONN_MODEL_H

#include <unirec/ur_time.h>

/* Events (Exceptions) */
struct ModelInitialised : public std::exception
{
  const char * what() const throw ()
  {
    return "The model has been successfully initialised.";
  }

};

/* Model interface */
class BLEConnModel
{
public:
  virtual void receivedAdvAt(ur_time_t time) = 0;
  
  virtual bool isReady(void) = 0;
};

#endif
