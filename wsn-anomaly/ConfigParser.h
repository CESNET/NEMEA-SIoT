/**
 * \file ConfigParser.h
 * \brief Parse file with configuration of unirec field.
 * \author Dominik Soukup <soukudom@fit.cvut.cz>
 * \date 2018
**/
/* NOTE: Detailed explanation of this code is in documentation. */

/*
 * Copyright (C) 2018 CESNET
 *
 * LICENSE TERMS
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * This software is provided ``as is'', and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include "Conversion.h"
#include "INIReader.h"

using namespace std;

// Number of values in meta structure
#define DYNAMIC 15

/*
 The following enums are used for accessing the created structures after parsing the configuraiton file.
*/

// Parameters and fields for each value in profile.
enum localValues {SOFT_MIN, SOFT_MAX, HARD_MIN, HARD_MAX, SOFT_PERIOD, GROW_UP, GROW_DOWN, S_MIN_LIMIT, S_MAX_LIMIT};

// Parameters for every analyzed unirec field.
enum general {SERIES_LENGTH, LEARNING_LENGTH, IGNORE_LENGTH, STORE_MODE, PERIODIC_CHECK, PERIODIC_INTERVAL, EXPORT_INTERVAL};

// Parameters and fields used in analyse process
enum meta {MOV_AVERAGE, MOV_VARIANCE, MOV_MEDIAN, AVERAGE, SX, SX2, PREV_VALUE, NEW_VALUE, LAST_TIME, ROTATE, CHECKED_FLAG, OLDEST_VALUE, NEW_ORIG_VALUE, CHANGE_PERIOD, CNT};

/*
 Parse file with configuration of unirec field.
*/
class ConfigParser{
    public:
        /*
        * Constructor
        * /param[in] configFile Name of configuration file
        */
        ConfigParser(string configFile);
        /*
        * Destructor
        */
        virtual ~ConfigParser();
        /*
        * Method that returns parsed structure from configuration file
        */
        map<string, map<uint64_t, map<string, vector<string> > > > getSeries();
        /*
        * Parse data in configuration file in ini format into series structure
        */
        int parseIniFile();
        /*
        * Parse data in configuration file into series structure
        */
        int parseConfFile();
        /*
        * Method for selecting configuration file format
        */
        void parseFile();
        /*
        * Method for veryfiing configuration values
        */
        int checkValue(string parsed_value, string key_name);
        /*
        * Method to split string based on delimiter
        */
        vector<string> parseString(string value, string delimiter);
    private:
        map<string, map<uint64_t, map<string, vector<string> > > > series; // parsed data from configuration file. Data sequence: unirec field, ur_id, subsection category (profile, profile items, export, general, metaData, metaProfile, profile), config params
        ifstream config; // configuration filename
        string config_filename; // configuration filename
        string main_key;     // Name of unirec field
        uint64_t main_id;    // Name of record ID   
        
        
};
