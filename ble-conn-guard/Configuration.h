#ifndef BCG_CONFIGURATION_H
#define BCG_CONFIGURATION_H

#include <map>
#include <set>
#include <string>

#include "INIReader.h"

#define NULL_BDADDR "00:00:00:00:00:00"

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

enum ReportMode { never, timed, always };

class DeviceConf final
{
public:
  explicit DeviceConf(const std::string bdaddr, const std::string report)
    : bdaddr(bdaddr)
  {
    if (report == "never")
      this->report = never;
    else if (report == "timed")
      this->report = timed;
    else
      this->report = always;
  }

  const std::string getBdaddr() const {
    return bdaddr;
  }

  const ReportMode getReportMode() const {
    return report;
  }

private:
  const std::string bdaddr;
  ReportMode  report;
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
        general = new DeviceConf(NULL_BDADDR,
          reader.Get("general", "report", "never"));
      } else {
        DeviceConf *dConf = new DeviceConf( (*it),
          reader.Get( (*it), "report", "always"));
        devices[(*it)] = dConf;
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

  const bool allowedConnection(const std::string device) {
    const DeviceConf *conf = getConf(device);
    
    if (conf->getReportMode() == never)
      return true;

    return false;
  }

private:
  const std::string configFile;
  DeviceConf *general = NULL;
  std::map<std::string, DeviceConf*> devices;
};

#endif
