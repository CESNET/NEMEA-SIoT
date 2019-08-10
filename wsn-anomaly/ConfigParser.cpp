/**
 * \file ConfigParser.cpp
 * \brief Parse file with configuration of unirec field.
 * \author Dominik Soukup <soukudom@fit.cvut.cz>
 * \date 2018
**/

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

#include "ConfigParser.h"

using namespace std;

// Constructor
ConfigParser::ConfigParser(string configFile) : config(configFile), config_filename(configFile) {}

// Select type of configuration file
void ConfigParser::parseFile(){
int err = 0;
    if (config.is_open()){
        err = parseIniFile();
        if (err){
           err = parseConfFile(); 
           if (err){
                cerr << "ERROR: Wrong format of configuration file" << endl;
                cerr << "NOTE: Read documentation and improve it!!" << endl;
            }
        }
    } else {
        cerr << "ERROR: Unable to open the configuration fle " << endl;
        cerr << "NOTE: Create configuration file!!" << endl;
    }
    // TODO: print/return success return code -> err == 0
    config.close();

}

vector<string> ConfigParser::parseString(string value, string delimiter){

    size_t pos = 0;
    string token;
    vector<string> parsedData;
    while ((pos = value.find(delimiter)) != string::npos) {
        token = value.substr(0, pos);
        parsedData.push_back(token);
        value.erase(0, pos + delimiter.length());
    }

    parsedData.push_back(value);
    return parsedData;
}

int ConfigParser::checkConfigRelations(){

    return 0;
}

int ConfigParser::checkSubsectionValue(string parsed_value, string key_name){
    try {
        // Check if parsed_value is floating-point datatype
        stod(parsed_value);
        // Also check if grace_perido is integer in the correct range
        if (key_name == "grace_period" && stoi(parsed_value) < 0 ){
            cerr << "ERROR: Grace period must be positive integer" << endl;
            return 2;
        }

    // In case of non-floating-point datatype verify default value
    } catch (const std::exception& e) {
        // Default value
        if (parsed_value == "-"){
            return 0;
        } else {
            cerr << "ERROR: Wrong value " << parsed_value << ". Check documentation for the right range." << endl;
            return 2;
        }
    }
}


int ConfigParser::checkSectionValue(string parsed_value, string key_name ){

    try {
        if ( (stoi(parsed_value) < 0 && "ignore" == key_name) || stoi(parsed_value) <= 0 ) {
            cerr << "ERROR: Wrong value " << parsed_value  << ". Check documentation for the right range." << endl;
            return 2;
        } else {
            // Add value to the data structure

            //TODO if keyname contais _ -> subsection
            series[main_key][main_id]["general"].push_back(parsed_value);
            return 0;
        }
    } catch (const std::exception& e) {
        cerr << "ERROR: The value " << parsed_value << " must be int" << endl;
        return 2;
    }

    // Add value to the data structure
    // series[main_key][main_id]["general"].push_back(parsed_value);
    // return 0;
}

// Parse ini configuration file
int ConfigParser::parseIniFile(){
    set<string> sections; // All specified sections in ini file
    vector<string> profile_items; // Parsed profile items
    //string main_key = "";   // UniRec field name
    string parsed_value = ""; // Parsed value for ini configuration file
    int check_result = 0; // Result of configuration value check
    //uint64_t main_id;    // Name of record ID
    string tmp_main_id;  // Tmp variable for the record ID
    

    // Parse ini configuration file
    INIReader reader(config_filename);
    if (reader.ParseError() < 0){
        cerr << "ERROR: Unable to load the configuration fle " << endl;
        return 1;
    }


    for (auto it=sections.begin(); it != sections.end(); ++it){

        // Parse the main section
        if ((*it).find(".") == string::npos ){
            main_key = *it;


            // Create main id
            tmp_main_id = reader.Get(*it,"id","-");
            // ID was not found -> use default value
            if (tmp_main_id == "-" ){
                main_id = 0;
            } else {
                // Check if sensor ID is in mac addr form
                if (tmp_main_id.find("-") != string::npos){
                    // Conver mac addr to hex int
                    tmp_main_id.erase(remove(tmp_main_id.begin(), tmp_main_id.end(), '-'), tmp_main_id.end());
                    istringstream iss(tmp_main_id);
                    iss >> hex >> main_id;
                } else {
                    istringstream iss(tmp_main_id);
                    iss >> main_id;
                }
            }

            // Prepare data structures
            for (int i=0; i < DYNAMIC; i++){
                series[main_key][main_id]["metaProfile"].push_back(to_string(0));
                series[main_key][main_id]["metaData"].push_back("x");
            }

            parsed_value = reader.Get(*it,"len","-");
            check_result = checkSectionValue(parsed_value,"len");
            if (check_result != 0 ){
                return check_result;
            }
            
            parsed_value = reader.Get(*it,"learn","-");
            check_result = checkSectionValue(parsed_value,"learn");
            if (check_result != 0 ){
                return check_result;
            }

            parsed_value = reader.Get(*it,"ignore","-");
            check_result = checkSectionValue(parsed_value,"ignore");
            if (check_result != 0 ){
                return check_result;
            }

            parsed_value = reader.Get(*it,"store","-");
            if (parsed_value != "store" || parsed_value != "simple"){
                cerr << "ERROR: Wrong value " << parsed_value  << ". Check documentation for the right range." << endl;
                return 2;
            }
        
            parsed_value = reader.Get(*it,"check","-");
            check_result = checkSectionValue(parsed_value,"check");
            if (check_result != 0 ){
                return check_result;
            }
            
            parsed_value = reader.Get(*it,"export","-");
            check_result = checkSectionValue(parsed_value,"export");
            if (check_result != 0 ){
                return check_result;
            }

            // Parsed separetely because of independed section
            parsed_value = reader.Get(*it,"profile","-");
            if (parsed_value == "-"){
                cerr << "ERROR: Profile keyword is required. Please follow configuraiton guide" << endl;
                return 2;
            }
            profile_items = parseString(parsed_value,",");
            if (profile_items.size() == 0 ){
                cerr << "ERROR: Profile items in profile keyword are wrongly formated. Expected delimiter is ,. Please follow configuration guide for more details" << endl;
                return 2;
            }
            for (auto it2=profile_items.begin(); it2 != profile_items.end(); ++it2){
                series[main_key][main_id]["profile"].push_back(*it2);
            } 

            parsed_value = reader.Get(*it,"export_fields","-");
            if (parsed_value == "-"){
                series[main_key][main_id]["export"].push_back(parsed_value);
            } else {
                profile_items = parseString(parsed_value,",");
                if (profile_items.size() == 0 ){
                    cerr << "ERROR: Export parameter is wrongly formated. Expected delimiter is ,. Please follow configuration guide for more details" << endl;
                    return 2;

                }
                for (auto it2=profile_items.begin(); it2 != profile_items.end(); ++it2){
                    series[main_key][main_id]["export"].push_back(*it2);
                } 
            }

        } else {
        // Parse subsection
            vector<string> tmp_subsection = parseString((*it),".");
            if (tmp_subsection.size() != 2){
                cerr << "ERROR: Subsection in the configuration file is wrongly named." << endl;
                return 2;
            }

            // Verify if found subsection configuration has proper keyword in profile key configured 
            if (find(profile_items.begin(), profile_items.end(), tmp_subsection[1]) != profile_items.end() ){
                parsed_value = reader.Get(*it,"soft_min","-");
                check_result = checkSubsectionValue(parsed_value, "soft_min");
                if (check_result != 0 ){
                    return check_result;
                }
                series[main_key][main_id][tmp_subsection[1]].push_back( parsed_value );

                parsed_value = reader.Get(*it,"soft_max","-");
                check_result = checkSubsectionValue(parsed_value, "soft_max");
                if (check_result != 0 ){
                    return check_result;
                }
                series[main_key][main_id][tmp_subsection[1]].push_back( parsed_value );

                parsed_value = reader.Get(*it,"hard_min","-");
                check_result = checkSubsectionValue(parsed_value, "hard_min");
                if (check_result != 0 ){
                    return check_result;
                }
                series[main_key][main_id][tmp_subsection[1]].push_back( parsed_value );

                parsed_value = reader.Get(*it,"hard_max","-");
                check_result = checkSubsectionValue(parsed_value, "hard_max");
                if (check_result != 0 ){
                    return check_result;
                }
                series[main_key][main_id][tmp_subsection[1]].push_back( parsed_value );

                parsed_value = reader.Get(*it,"grace_period","-");
                check_result = checkSubsectionValue(parsed_value, "grace_period");
                if (check_result != 0 ){
                    return check_result;
                }

                parsed_value = reader.Get(*it,"grow_up","-");
                check_result = checkSubsectionValue(parsed_value, "grow_up");
                if (check_result != 0 ){
                    return check_result;
                }
                series[main_key][main_id][tmp_subsection[1]].push_back( parsed_value );
    
                parsed_value = reader.Get(*it,"grow_down","-");
                check_result = checkSubsectionValue(parsed_value, "grow_down");
                if (check_result != 0 ){
                    return check_result;
                }
                series[main_key][main_id][tmp_subsection[1]].push_back( parsed_value );

                //TODO check if related section has been configured
                // Also check releation beween each other (min max values, ...)

                check_result = checkConfigRelations();
            }
        }
    }
    return 0;
}

// TODO: implement checks during parsion a configuration file
// Parse configuration file
int ConfigParser::parseConfFile(){
    if (config.is_open()){
        // Local tmp store variables
        string line;         // One line from configuration file
        string key;          // Name of config record (unirec field+id)
        //string main_key;     // Name of unirec field
        //uint64_t main_id;    // Name of record ID
        string tmp_main_id;  // Tmp variable for the record ID
        string value;        // Config params for one key
        string multi_key;    // Name of composite value
        string multi_value;  // Config params for one composite key
        string simple_value; // One item of multi value record
        size_t multi_item;   // Tmp local value for storing position in config line

        while(getline(config,line)){
            // Erase whitespace
            line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
            // Erase comments and empty lines
            if(line[0] == '#' || line.empty()) continue;
            // Parse config record key (ur_field + id)
            auto delimiter = line.find(":");
            key = line.substr(0, delimiter);
            multi_item = key.find(";");
            // Ur_field with id has been found
            if (multi_item != string::npos){
                main_key = key.substr(0, multi_item);
                tmp_main_id = key.substr(multi_item+1,delimiter);
                // Check if sensor ID is in mac addr form
                if (tmp_main_id.find("-") != string::npos){
                    // Conver mac addr to hex int
                    tmp_main_id.erase(remove(tmp_main_id.begin(), tmp_main_id.end(), '-'), tmp_main_id.end());
                    istringstream iss(tmp_main_id);
                    iss >> hex >> main_id;
                } else {
                    istringstream iss(tmp_main_id);
                    iss >> main_id;
                }
            // Key with no ID -> use default ID value
            } else {
                main_key = key;
                main_id = 0;
            }
            line.erase(0,delimiter+1);

            // Insert ur_field to the map
            while (delimiter != string::npos){
                auto last_delimiter = delimiter;
                // Parse next config field
                delimiter = line.find(";");
                value = line.substr(0, delimiter);
                multi_item = value.find("(");
                // Composite key has been found -> name(param1, param2, ...,)
                if (multi_item != string::npos){
                    // Parse and save to the proper index
                    multi_key = value.substr(0, multi_item);
                    value.erase(0,multi_item+1);
                    multi_item = value.find(")");
                    value = value.substr(0, multi_item);
                    // Parse values for composite key
                    size_t multi_value = value.find(",");
                    // Multi key (more than one item) -> value parsing needed
                    if (multi_value != string::npos){
                        while (multi_value != string::npos){
                            simple_value = value.substr(0,multi_value);
                            series[main_key][main_id][multi_key].push_back(simple_value);
                            value.erase(0,multi_value+1);
                            multi_value = value.find(",");
                        }
                    // Simple value -> save directly
                    } else {
                        series[main_key][main_id][multi_key].push_back(value);
                        value.erase(0,multi_item+1);
                    }
                    line.erase(0,delimiter+1);
                // General key has been found -> just value without brackets
                } else {
                    series[main_key][main_id]["general"].push_back(value);
                    line.erase(0,delimiter+1);
                }
            }
            // Insert dynamic initialization values
            for (int i=0; i < DYNAMIC; i++){
                series[main_key][main_id]["metaProfile"].push_back(to_string(0));
                series[main_key][main_id]["metaData"].push_back("x");
            }
        }
    } else {
        cerr << "ERROR: Unable to open the configuration fle " << endl;
        cerr << "NOTE: Create configuration file!!" << endl;
    }
    config.close();
    return 0;
}

// Destructor
ConfigParser::~ConfigParser() = default;

// Getter function
map<string, map<uint64_t, map<string, vector<string> > > > ConfigParser::getSeries(){
    return series;
}
