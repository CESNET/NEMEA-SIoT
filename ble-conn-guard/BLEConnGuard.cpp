#include <iostream>
#include <string>

#include "Configuration.h"

#define DEFAULT_CONFIG "ble-conn-guard.ini"

// #define MODULE_BASIC_INFO(BASIC) \
	BASIC("BLE Connection Guard", \
		"This module receives UniRec containing information about BLE connections" \
		"and sends alerts if the connection is not allowed according to setting.", \
    1, 1)

// #define MODULE_PARAMS(PARAM) \
	PARAM('I', "ignore-in-eof", "Do not terminate on incomming termination message.", no_argument, "none") \
	PARAM('c', "config", "Use this configuration file. (Default is ./ble-conn-guard.ini)", required_argument, "none")

int main()
{
  int retval = 0;
  Configuration* config = NULL;
  
  try {
    config = new Configuration(DEFAULT_CONFIG);
  } catch (IOError& e) {
    std::cerr << e.what() << std::endl;
  } catch (ParseError& e) {
    std::cerr << e.what() << std::endl;
  }

  std::cout << "Default [" << config->getDefaultConf()->getBdaddr() << "]: ";
  std::cout << config->getDefaultConf()->getReportMode() << std::endl;

  std::cout << "[" << config->getConf("6d:73:5a:f3:7b:98")->getBdaddr() << "]: ";
  std::cout << config->getConf("6d:73:5a:f3:7b:98")->getReportMode() << std::endl;

  std::cout << "[" << config->getConf("00:73:5a:f3:7b:98")->getBdaddr() << "]: ";
  std::cout << config->getConf("00:73:5a:f3:7b:98")->getReportMode() << std::endl;
  
  std::cout << "[" << config->getConf("00:00:5a:f3:7b:98")->getBdaddr() << "]: ";
  std::cout << config->getConf("00:00:5a:f3:7b:98")->getReportMode() << std::endl;
  
  delete config;
  return retval;
}
