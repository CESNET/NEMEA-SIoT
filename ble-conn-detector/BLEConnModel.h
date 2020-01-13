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

struct ConnectionDetected : public std::exception
{
  const ur_time_t timestamp;
  const uint64_t  duration;
  
  ConnectionDetected(ur_time_t timestamp, uint64_t duration)
    : timestamp(timestamp)
    , duration(duration)
  {}

  const char * what() const throw ()
  {
    return "A connection has been detected.";
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
