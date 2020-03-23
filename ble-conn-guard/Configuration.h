#ifndef BCG_CONFIGURATION_H
#define BCG_CONFIGURATION_H

#include <ctime>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "INIReader.h"

#define NULL_BDADDR "00:00:00:00:00:00"


/*
 * Holds from and to dates for timed reports.
 */
struct TimeSpan
{
  int from_hour; /* hours, range 0 to 23 */
  int from_min;  /* minutes, range 0 to 60 */

  int to_hour; /* hours, range 0 to 23 */
  int to_min;  /* minutes, range 0 to 60 */
};

/* Exceptions */
struct IOError
{
  const std::string what_arg;

  explicit IOError(const std::string what_arg) : what_arg(what_arg) {}

  virtual const std::string what() const {
    return what_arg;
  }
};

struct ParseError
{
  const std::string what_arg;

  explicit ParseError(const std::string what_arg) : what_arg(what_arg) {}

  virtual const std::string what() const {
    return what_arg;
  }
};

/* Device configuration */

/*
 * Holds reporting modes:
 *  never = do not report
 *  timed = report between start and stop times
 *  always = report always
 */
enum ReportMode { never, timed, trigger, always };

/*
 * Converts string with ReportMode to enum, default value is "always"
 */
inline const enum ReportMode stringToReportMode(const std::string report) {
  if (report == "never")
    return never;
  else if (report == "timed")
    return timed;
  else if (report == "trigger")
    return trigger;
  else // always is default
    return always;
}

struct DeviceConf final
{
public:
  explicit DeviceConf(
    const std::string bdaddr,
    const ReportMode report,
    const TimeSpan ts
  )
    : bdaddr(bdaddr)
    , report(report)
    , ts(ts)
  {}

  const std::string *getBdaddr() const {
    return &bdaddr;
  }

  const ReportMode *getReportMode() const {
    return &report;
  }

  const TimeSpan *getTimeSpan() const {
    return &ts;
  }

private:
  const std::string bdaddr;
  const ReportMode  report;
  const TimeSpan    ts;
};

/* Main configuration class */
class Configuration
{
public:
  Configuration(const std::string configFile) : configFile(configFile) {
    INIReader reader(configFile);
    
    if (reader.ParseError() < 0) {
      throw IOError("Can't load the config file \"" + configFile + "\".");
    } else if (reader.ParseError() != 0) {
      //FIXME: Probably useless, was unable to cause parse error
      throw ParseError("When parsing configuration file \"" + configFile + "\" "
        + "Error occured on line " + std::to_string(reader.ParseError()) + ".");
    }

    std::set<std::string> sections = reader.Sections();
    for (auto it = sections.begin(); it != sections.end(); ++it) {

      if ((*it) == "general") {
        const ReportMode report = stringToReportMode(
          reader.Get("general", "report", "never"));
        
        struct TimeSpan ts;
        std::size_t pos;

        const std::string start_time = reader.Get("general", "start_time", "00:00");
        pos = start_time.find(':');
        ts.from_hour = std::stoi(start_time.substr(0,pos));
        ts.from_min  = std::stoi(start_time.substr(pos + 1));

        const std::string end_time = reader.Get("general", "end_time", "00:00");
        pos = end_time.find(':');
        ts.to_hour = std::stoi(end_time.substr(0,pos));
        ts.to_min  = std::stoi(end_time.substr(pos + 1));

        general = new DeviceConf(NULL_BDADDR, report, ts);
      } else {
        const ReportMode report = stringToReportMode(
          reader.Get((*it), "report", "always"));
        
        struct TimeSpan ts;
        std::size_t pos;

        const std::string start_time = reader.Get((*it), "start_time", "00:00");
        pos = start_time.find(':');
        ts.from_hour = std::stoi(start_time.substr(0,pos));
        ts.from_min  = std::stoi(start_time.substr(pos + 1));

        const std::string end_time = reader.Get((*it), "end_time", "00:00");
        pos = end_time.find(':');
        ts.to_hour = std::stoi(end_time.substr(0,pos));
        ts.to_min  = std::stoi(end_time.substr(pos + 1));

        devices[(*it)] = new DeviceConf((*it), report, ts);
      }
    }

  }

  ~Configuration() {
    if (general != NULL)
      delete general;

    for (auto const& dev : devices) {
      delete dev.second;
    }
  }

  const DeviceConf *getDefaultConf() {
    return general;
  }
  
  const DeviceConf *getConf(const std::string device) {
    std::map<std::string, DeviceConf*>::iterator it;
    it = devices.find(device);
    if (it != devices.end())
      return it->second;
    else
      return getDefaultConf();
  }

  const bool allowedConnection(const std::string device, const std::tm *time) {
    const DeviceConf *conf = getConf(device);
    
    if (*(conf->getReportMode()) == never)
      return true;

    if (*(conf->getReportMode()) == timed) {
      const TimeSpan *ts = conf->getTimeSpan();

      if (ts->from_hour < ts->to_hour) {
        if (time->tm_hour < ts->from_hour || time->tm_hour > ts->to_hour)
          return true;

        if (time->tm_hour > ts->from_hour && time->tm_hour < ts->to_hour)
          return false;

        if (time->tm_hour == ts->from_hour) {
          return (time->tm_min < ts->from_min);
        }

        if (time->tm_hour == ts->to_hour) {
          return (time->tm_min > ts->to_min);
        }

        // Should never reach this!
        throw "Runtime error: Timed report reached invalid point.";

      } else if (ts->from_hour == ts->to_hour) {

        return (time->tm_min < ts->from_min || time->tm_min > ts->to_min);

      } else {
        if (time->tm_hour < ts->from_hour && time->tm_hour > ts->to_hour)
          return true;

        if (time->tm_hour > ts->from_hour || time->tm_hour < ts->to_hour)
          return false;

        if (time->tm_hour == ts->from_hour)
          return (time->tm_min > ts->from_min);

        if (time->tm_hour == ts->to_hour);
          return (time->tm_min < ts->to_min);
        
        // Should never reach this!
        throw "Runtime error: Timed report reached invalid point.";

      }
    }

    if (*(conf->getReportMode()) == trigger) {
      return !_trigger;
    }

    return false;
  }

  bool setTrigger() {
    _trigger = true;
    return _trigger;
  }

  bool unsetTrigger() {
    _trigger = false;
    return !_trigger;
  }

private:
  const std::string configFile;
  DeviceConf *general = NULL;
  std::map<std::string, DeviceConf*> devices;
  bool _trigger = false;
};

#endif
