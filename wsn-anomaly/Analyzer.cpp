/**
 * \file Analyzer.cpp
 * \brief Analyze time series data based on configuration
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

#include "Analyzer.h"
#include "ConfigParser.h"

// Default constructor
Analyzer::Analyzer() = default;

// Param constructor
Analyzer::Analyzer(map<string, map<uint64_t, map<string, vector<string> > > > meta_data,int verbose): series_meta_data(meta_data), verbose(verbose) {this->default_interface = 0;}

// Default destructor
Analyzer::~Analyzer() = default;

// Setters for trap interfaces
void Analyzer::setAlertInterface(trap_ctx_t *alert_ifc, ur_template_t *alert_template, void *data_alert){
    this->alert_ifc = alert_ifc;
    this->alert_template = alert_template;
    this->data_alert = data_alert;
}

void Analyzer::setExportInterface(trap_ctx_t *export_ifc, ur_template_t **export_template, void **data_export, map<int, vector<string> > ur_export_fields){

    this->export_ifc = export_ifc;
    this->export_template = export_template;
    this->data_export = data_export;
    this->ur_export_fields = ur_export_fields;
}

/*
 * BEGIN CALCALULATION METHODS
 */
double Analyzer::getMovingMedian(map<uint64_t,vector<double> >::iterator &sensor_it, map<string, map<uint64_t, map<string, vector<string> > > >::iterator &meta_it, string &ur_field, uint64_t *ur_id){
    // Moving median calculation

    // First value exception
    if (sensor_it->second.size() == 1){
        return sensor_it->second.back();
    }
    int series_length = stoi (meta_it->second[getMetaID(meta_it,ur_id)]["general"][SERIES_LENGTH],nullptr);
    double new_value = stod (meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][NEW_VALUE],nullptr);
    double old_value = stod (meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][OLDEST_VALUE],nullptr);
    double median = 0;
    int index = 0;

    // Insert the new value
    if (series_length > median_window[ur_field][sensor_it->first].size()){
        // Push back
        median_window[ur_field][sensor_it->first].push_back(new_value);
    } else {
        // Vector is full -> replace the oldest value
        index = 0;
        for (auto elem: median_window[ur_field][sensor_it->first]){
            if (elem == old_value){
                median_window[ur_field][sensor_it->first][index] = new_value;
                break;
            }
            index++;
        }
    }

    // Calculate moving median value
    size_t size = median_window[ur_field][sensor_it->first].size();

    // Sort modified vector by the new value
    sort(median_window[ur_field][sensor_it->first].begin(), median_window[ur_field][sensor_it->first].end());

    if (size  % 2 == 0) {
        median = (median_window[ur_field][sensor_it->first][size / 2 - 1] + median_window[ur_field][sensor_it->first][size / 2]) / 2;
    }
    else {
        median = median_window[ur_field][sensor_it->first][size / 2];
    }
    return median;
}

double Analyzer::getOverallAverage(map<uint64_t,vector<double> >::iterator &sensor_it, map<string, map<uint64_t, map<string, vector<string> > > >::iterator &meta_it, string meta_id, uint64_t *ur_id){
    // Update number of received items
    double cnt = stod (meta_it->second[getMetaID(meta_it,ur_id)][meta_id][CNT],nullptr);
    cnt++;
    meta_it->second[getMetaID(meta_it,ur_id)][meta_id][CNT] = to_string(cnt);

    double new_value = sensor_it->second.back();
    // First value exception
    if (sensor_it->second.size() == 1){
        return new_value;
    }
    double delta_average = stod (meta_it->second[getMetaID(meta_it,ur_id)][meta_id][AVERAGE],nullptr);
    delta_average = ((new_value + ( (cnt-1) * delta_average )) / ( cnt )) ;
    return delta_average;
}

pair<double, double> Analyzer::getMovingAverageAndVariance(string &ur_field, uint64_t *ur_id, map<string, map<uint64_t, map<string, vector<string> > > >::iterator &meta_it, map<uint64_t, vector<double> >::iterator &sensor_it, string meta_id){
    // Alg source: https://www.dsprelated.com/showthread/comp.dsp/97276-1.php
    map<uint64_t, vector<double> >::iterator x_it;
    map<uint64_t, vector<double> >::iterator x2_it;

    int series_length = stoi (meta_it->second[getMetaID(meta_it,ur_id)]["general"][SERIES_LENGTH],nullptr);
    double new_value = sensor_it->second.back();
    double variance = 0;
    double average = 0;
    double sx = 0;
    double sx2 = 0;

    // Initialize time window
    if (x[ur_field][*ur_id].size() < series_length ){
        // Add new data to time window
        x[ur_field][*ur_id].push_back(new_value);
        x2[ur_field][*ur_id].push_back(new_value*new_value);
        series_length = x[ur_field][*ur_id].size();

        // Count sum of values in window
        meta_it->second[getMetaID(meta_it,ur_id)][meta_id][SX] = to_string(new_value + stod(meta_it->second[getMetaID(meta_it,ur_id)][meta_id][SX],nullptr));
        meta_it->second[getMetaID(meta_it,ur_id)][meta_id][SX2] = to_string(new_value*new_value+ stod(meta_it->second[getMetaID(meta_it,ur_id)][meta_id][SX2],nullptr));

    // Time window is full -> rotate and modify meta structures
    } else {
        // Change values in time window
        rotate(x[ur_field][*ur_id].begin(), x[ur_field][*ur_id].begin()+1, x[ur_field][*ur_id].end());
        rotate(x2[ur_field][*ur_id].begin(), x2[ur_field][*ur_id].begin()+1, x2[ur_field][*ur_id].end());

        double new_x = new_value;
        double new_x2 = new_value*new_value;

        double y = x[ur_field][*ur_id].back();
        double y2 = x2[ur_field][*ur_id].back();

        x[ur_field][*ur_id].back() = new_x;
        x2[ur_field][*ur_id].back() = new_x2;

        meta_it->second[getMetaID(meta_it,ur_id)][meta_id][SX] = to_string(stod(meta_it->second[getMetaID(meta_it,ur_id)][meta_id][SX],nullptr) + new_x - y);
        meta_it->second[getMetaID(meta_it,ur_id)][meta_id][SX2] = to_string(stod(meta_it->second[getMetaID(meta_it,ur_id)][meta_id][SX2],nullptr) + new_x2 - y2);
    }
        // Do a calculation and return result
        sx = stod(meta_it->second[getMetaID(meta_it,ur_id)][meta_id][SX],nullptr);
        sx2 = stod(meta_it->second[getMetaID(meta_it,ur_id)][meta_id][SX2],nullptr);

        average = sx/series_length;
        // NaN protection
        if (series_length == 1){
            variance = 0;
        } else {
            variance = (series_length*sx2 - (sx*sx)) / (series_length*(series_length-1));
        }

        return pair<double,double> (average, variance);
}
/*
 * END CALCALULATION METHODS
 */

/*
 * BEGIN TIME SERIES SERVICE AND INIT METHODS
 */

// Store data in data series based on configured values
pair<double, double> Analyzer::pushData(double *ur_time, double *ur_data, uint64_t *ur_id, map<string, map<uint64_t, map<string, vector<string> > > >::iterator &meta_it, map<uint64_t, vector<double> >::iterator &sensor_it, string meta_id){
    double new_value = 0;
    double old_value = 0;
    string store_mode = meta_it->second[getMetaID(meta_it,ur_id)]["general"][STORE_MODE];
    int series_length = stoi (meta_it->second[getMetaID(meta_it,ur_id)]["general"][SERIES_LENGTH],nullptr);

    // Save data based on store mode
    if (store_mode == "simple"){
        new_value = *ur_data;
    } else if (store_mode == "delta"){
        double prev_value = stod (meta_it->second[getMetaID(meta_it,ur_id)][meta_id][PREV_VALUE],nullptr);
        new_value = *ur_data - prev_value;
        meta_it->second[getMetaID(meta_it,ur_id)][meta_id][PREV_VALUE] = to_string(*ur_data);
    } else {
        cerr << "ERROR: Unknown mode" << endl;
    }

    // Insert new value
    if (series_length > sensor_it->second.size()){
        // Push back
        sensor_it->second.push_back(new_value);
        old_value = sensor_it->second[0];
    } else {
        //back() -> vector is full
        old_value = sensor_it->second.back();
        sensor_it->second.back() = new_value;
    }
    // New value field is useful just for alert detection methods -> not relevant in metaProfile
    meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][NEW_VALUE] = to_string(new_value);
    meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][LAST_TIME] = to_string(*ur_time);
    meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][OLDEST_VALUE] = to_string(old_value);
    meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][NEW_ORIG_VALUE] = to_string(*ur_data);
    if (verbose >= 1){
        cout << "VERBOSE: New data pushed: " << new_value << endl;
    }
    return pair<double, double>(new_value, old_value);
}

void Analyzer::modifyMetaData(string &ur_field, uint64_t *ur_id ,map<string, map<uint64_t, map<string, vector<string> > > >::iterator &meta_it, map<uint64_t, vector<double> >::iterator sensor_it, string meta_id){
    int flag = 0; // Decision flag

    // Determine profile values
    for (auto profile_values:  meta_it->second[getMetaID(meta_it,ur_id)]["profile"]){
        if ( profile_values == "MOVING_MEDIAN" ){
            // Moving median method
            meta_it->second[getMetaID(meta_it,ur_id)][meta_id][MOV_MEDIAN] = to_string(getMovingMedian(sensor_it,meta_it,ur_field,ur_id));
        } else if (profile_values == "MOVING_AVERAGE" || profile_values == "MOVING_VARIANCE") {
            // Moving varinace and average method
            // Skip unnecessary calls
            if (flag == 1 ){
                continue;
            }
            pair<double, double> determine_values = getMovingAverageAndVariance(ur_field, ur_id, meta_it, sensor_it,meta_id);
            meta_it->second[getMetaID(meta_it,ur_id)][meta_id][MOV_AVERAGE] = to_string(determine_values.first);
            meta_it->second[getMetaID(meta_it,ur_id)][meta_id][MOV_VARIANCE] = to_string(determine_values.second);
            flag = 1;
        } else if (profile_values == "AVERAGE"){
            // Overall average method
            meta_it->second[getMetaID(meta_it,ur_id)][meta_id][AVERAGE] = to_string(getOverallAverage(sensor_it, meta_it,meta_id,ur_id));
        }
    }
}

int Analyzer::initSeries(string &ur_field, uint64_t *ur_id, double *ur_data, double *ur_time){
    map<string, map<uint64_t, map<string, vector<string> > > >::iterator meta_it;
    map<uint64_t, vector<double> >::iterator sensor_it;
    uint64_t localID = 0;

    // Test if meta information exist
    meta_it = series_meta_data.find(ur_field);

    if (meta_it != series_meta_data.end()){

        // Test if meta information with the proper ID exist
        localID = getMetaID(meta_it,ur_id);
        if (meta_it->second.find(localID) == meta_it->second.end() ){
            if (verbose >= 1){
                cout << "VERBOSE: Ignore field with id: " << localID << endl;
            }
            return 6;
        }
        // Cast necessary meta info values
        int learning_length = stoi (meta_it->second[getMetaID(meta_it,ur_id)]["general"][LEARNING_LENGTH],nullptr);
        int series_length = stoi (meta_it->second[getMetaID(meta_it,ur_id)]["general"][SERIES_LENGTH],nullptr);
        int rotate_cnt = stoi (meta_it->second[getMetaID(meta_it,ur_id)]["metaProfile"][ROTATE],nullptr);
        int ignore_cnt = stoi (meta_it->second[getMetaID(meta_it,ur_id)]["general"][IGNORE_LENGTH],nullptr);

        if (ignore_cnt > 0){
            meta_it->second[getMetaID(meta_it,ur_id)]["general"][IGNORE_LENGTH] = to_string(--ignore_cnt);
            if (verbose >= 0){
                cout << "VERBOSE: Ignoring phase" << endl;
            }
            return 5;
        }
        // Test if sensor id exists
        sensor_it = control[ur_field].find(*ur_id);
        if ( sensor_it != control[ur_field].end() ){

            // Learning profile phase
            if ( learning_length > sensor_it->second.size() + rotate_cnt){
                if (verbose >= 0){
                    cout << "VERBOSE: Series created -> learning phase" << endl;
                }
                if (series_length > sensor_it->second.size()){
                    if (verbose >= 0){
                        cout << "VERBOSE: Push new value" << endl;
                    }
                    // Push data
                    pushData(ur_time, ur_data, ur_id, meta_it, sensor_it, "metaProfile");
                    // Modify profile values
                    modifyMetaData(ur_field,ur_id,meta_it, sensor_it, "metaProfile");
                    return 4;
                // Rotate values in learning phase
                } else {
                    if (verbose >= 0){
                        cout << "VERBOSE: Rotate values" <<endl;
                    }
                    rotate( sensor_it->second.begin(), sensor_it->second.begin()+1, sensor_it->second.end());
                    // Push data
                    pushData(ur_time, ur_data, ur_id, meta_it, sensor_it, "metaProfile");
                    rotate_cnt++;
                    meta_it->second[getMetaID(meta_it,ur_id)]["metaProfile"][ROTATE] = to_string(rotate_cnt);
                    // Modify profile values
                    modifyMetaData(ur_field,ur_id,meta_it, sensor_it, "metaProfile");
                    return 3;
                }
            // Learning phase has been finished
            } else {
                // Check if init phase is finished for the first time and copy init values
                if ( meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][MOV_AVERAGE] == "x"){
                    if (verbose >= 0){
                        cout << "VERBOSE: learning phase finished -> start analyzing" << endl;
                    }

                    meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][MOV_AVERAGE] = meta_it->second[getMetaID(meta_it,ur_id)]["metaProfile"][MOV_AVERAGE];
                    meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][MOV_VARIANCE] = meta_it->second[getMetaID(meta_it,ur_id)]["metaProfile"][MOV_VARIANCE];
                    meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][MOV_MEDIAN] = meta_it->second[getMetaID(meta_it,ur_id)]["metaProfile"][MOV_MEDIAN];
                    meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][AVERAGE] = meta_it->second[getMetaID(meta_it,ur_id)]["metaProfile"][AVERAGE];
                    meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][SX] = meta_it->second[getMetaID(meta_it,ur_id)]["metaProfile"][SX];
                    meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][SX2] = meta_it->second[getMetaID(meta_it,ur_id)]["metaProfile"][SX2];
                    meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][PREV_VALUE] = meta_it->second[getMetaID(meta_it,ur_id)]["metaProfile"][PREV_VALUE];
                    meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][CNT] = meta_it->second[getMetaID(meta_it,ur_id)]["metaProfile"][CNT];
                }
                return 0;
            }
        // Sensor id not found - create a new record
        } else {
            vector<double> tmp;
            string store_mode = meta_it->second[getMetaID(meta_it,ur_id)]["general"][STORE_MODE];
            // Init variables by zeros in case of delta store mode
            if (store_mode == "delta"){
                tmp.push_back(0);
            // Init variables by first received value
            } else{
                tmp.push_back(*ur_data);
            }
            control[ur_field].insert(pair<uint64_t, vector<double> >(*ur_id,tmp));
            meta_it->second[getMetaID(meta_it,ur_id)]["metaProfile"][PREV_VALUE] = to_string(*ur_data);
            sensor_it = control[ur_field].find(*ur_id);
            // Modify profile values
            modifyMetaData(ur_field,ur_id,meta_it, sensor_it, "metaProfile");
            if (verbose >= 0){
                cout << "VERBOSE: New record has been created" << endl;
            }
            return 2;
        }
    } else {
        if (verbose >= 1){
            cout << "VERBOSE: Field " << ur_field <<  " meta specification missig. Skipping... " << endl;
        }
        return 1;
    }
}

/*
 * END TIME SERIES SERVICE AND INIT METHODS
 */

/*
 * BEGIN TIME SERIES PROCESS
 */

// Print series data
void Analyzer::printSeries(string &ur_field, uint64_t *ur_id){
    uint64_t localID = 0;
    cout << "field: " << ur_field << endl;
    for (auto element : control[ur_field]){
            cout << " ID value: " << element.first << endl;
            cout << "  Time series values: " << endl;
            cout << "   ";
            for (auto elem: element.second){
                cout << elem << ", ";
            }
            cout << endl;

            if (verbose >= 1){
                cout << "   MOV_AVERAGE, MOV_VARIANCE, MOV_MEDIAN, AVERAGE, SX, SX2, PREV_VALUE, NEW_VALUE, LAST_TIME, ROTATE, CHECKED_FLAG, OLDEST_VALUE, NEW_ORIG_VALUE, CHANGE_PERIOD" << endl;
            } else if (verbose == 0){
                cout << "   MOV_AVERAGE, MOV_VARIANCE, MOV_MEDIAN, AVERAGE"<< endl;
            }
            cout << "   ";

            // Set appropriate sensor ID
            localID = element.first;
            if (series_meta_data[ur_field].size() == 1){
                if (series_meta_data[ur_field].find(0) != series_meta_data[ur_field].end()){
                    localID = 0;
                }
            }

            if (verbose >=  1){
                cout << "Base profile: " << endl;
                cout << "    ";
                for (auto meta: series_meta_data[ur_field][localID]["metaProfile"] ){
                    cout.precision(1);
                    cout << fixed << meta << ", ";
                }
                cout << endl;
                cout << "   ";
                cout << "Actual profile: " << endl;
                cout << "    ";
                for (auto meta: series_meta_data[ur_field][localID]["metaData"] ){
                    cout.precision(1);
                    cout << fixed << meta << ", ";
                }
                cout << endl;
            }

            if (verbose ==  0){
                cout << "Base profile: " << endl;
                cout << "    ";
                cout.precision(1);
                cout << fixed << series_meta_data[ur_field][localID]["metaProfile"][MOV_AVERAGE] << ", " << series_meta_data[ur_field][localID]["metaProfile"][MOV_VARIANCE] << ", " << series_meta_data[ur_field][localID]["metaProfile"][MOV_MEDIAN] << ", " << series_meta_data[ur_field][localID]["metaProfile"][AVERAGE] << endl;

                cout << "   ";
                cout << "Actual profile: " << endl;
                cout << "    ";
                cout.precision(1);
                cout << fixed << series_meta_data[ur_field][localID]["metaData"][MOV_AVERAGE] << ", " << series_meta_data[ur_field][localID]["metaData"][MOV_VARIANCE] << ", " << series_meta_data[ur_field][localID]["metaData"][MOV_MEDIAN] << ", " << series_meta_data[ur_field][localID]["metaData"][AVERAGE] << endl;
            }
    }
    cout << endl;
}

// Get field index for connected keyword
int Analyzer::getIndex(string name){
    if (name == "MOVING_MEDIAN"){
        return MOV_MEDIAN;
    } else if (name == "MOVING_AVERAGE"){
        return MOV_AVERAGE;
    } else if (name == "MOVING_VARIANCE"){
        return MOV_VARIANCE;
    } else if (name == "AVERAGE"){
        return AVERAGE;
    } else if (name == "new_value"){
        return NEW_VALUE;
    } else {
        return -1;
    }

}
// Check if ID value was defined in configuration file or not
uint64_t Analyzer::getMetaID(map<string, map<uint64_t, map<string, vector<string> > > >::iterator &meta_it,  uint64_t *ur_id){
    if (meta_it->second.size() == 1){
        // If the only key is 0 return true -> default key
        if (meta_it->second.find(0) != meta_it->second.end() && (this->default_interface == 0 || this->default_interface == *ur_id)){
            this->default_interface = *ur_id;
            return 0;
        }
    }
    return *ur_id;
}

void Analyzer::addAlert(string & profile_name, string alert_message, map<string, vector<string> > & alert_str){
    // Find key
    auto alert_it = alert_str.find(profile_name);
    if (alert_it != alert_str.end()){
        alert_it->second.push_back(alert_message);
    } else {
        vector<string> tmp;
        tmp.push_back(alert_message);
        alert_str.insert(pair<string, vector<string> >(profile_name,tmp));
    }
}

void Analyzer::dataLimitCheck(map<string, map<uint64_t, map<string, vector<string> > > >::iterator &meta_it, string ur_field, uint64_t *ur_id, double *ur_time ,double *ur_data, map<string,vector<string> > &alert_str){
    double err = 0;
    string alert_message;

    for (auto profile_values: meta_it->second[getMetaID(meta_it,ur_id)]["profile"]){
        // Test if soft limits are set
        // Soft min, max limits are dependent -> test for soft min is ok
        if (meta_it->second[getMetaID(meta_it,ur_id)][profile_values][SOFT_MIN] != "-"){

            // Soft limit test
            // Soft limit min
            if (stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][getIndex(profile_values)],nullptr)  < stod(meta_it->second[getMetaID(meta_it,ur_id)][profile_values][SOFT_MIN],nullptr) ){
                // Add counter or create alert message if the counter is above soft period
                if ( stoi (meta_it->second[getMetaID(meta_it,ur_id)][profile_values][S_MIN_LIMIT],nullptr) > stod(meta_it->second[getMetaID(meta_it,ur_id)][profile_values][SOFT_PERIOD]) ){
                    if (verbose >= 0){
                        cout << "VERBOSE: ALERT: Lower soft limit" << endl;
                    }
                    // Count exact error value 
                    err = stod (meta_it->second[getMetaID(meta_it,ur_id)][profile_values][SOFT_MIN],nullptr) - stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][getIndex(profile_values)],nullptr); 
                    // Alert alert messaga (caption) and add alert code
                    alert_message = "The device " + to_string(*ur_id) + " exceeded the minimal soft limit by " + to_string(err) + "3";
                    addAlert(profile_values, alert_message, alert_str);
                } else{
                    meta_it->second[getMetaID(meta_it,ur_id)][profile_values][S_MIN_LIMIT] = to_string(stoi (meta_it->second[getMetaID(meta_it,ur_id)][profile_values][S_MIN_LIMIT],nullptr) + 1);
                }
            } else {
                // Reset counter
                meta_it->second[getMetaID(meta_it,ur_id)][profile_values][S_MIN_LIMIT] = "0";
            }

            // Soft limit max
            if (stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][getIndex(profile_values)],nullptr)  > stod(meta_it->second[getMetaID(meta_it,ur_id)][profile_values][SOFT_MAX],nullptr) ){
                // Add counter
                if ( stoi (meta_it->second[getMetaID(meta_it,ur_id)][profile_values][S_MAX_LIMIT],nullptr) > stod(meta_it->second[getMetaID(meta_it,ur_id)][profile_values][SOFT_PERIOD]) ){
                    if (verbose >= 0){
                        cout <<  "VERBOSE: ALERT: Higher soft limit" << endl;
                    }
                    // Count exact error value 
                    err = stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][getIndex(profile_values)],nullptr) - stod(meta_it->second[getMetaID(meta_it,ur_id)][profile_values][SOFT_MAX],nullptr);
                    // Alert alert messaga (caption) and add alert code
                    alert_message = "The device " + to_string(*ur_id) + " exceeded the maximal soft limit by " + to_string(err) + "4";
                    addAlert(profile_values, alert_message, alert_str);
                } else{
                    meta_it->second[getMetaID(meta_it,ur_id)][profile_values][S_MAX_LIMIT] = to_string(stoi (meta_it->second[getMetaID(meta_it,ur_id)][profile_values][S_MAX_LIMIT],nullptr) + 1);
                }
            } else {
                // Reset counter
                meta_it->second[getMetaID(meta_it,ur_id)][profile_values][S_MAX_LIMIT] = "0";
            }
        }

        // Test if hard limits are set
        // Hard min, max limits are dependent -> test for hard min is ok
        if (meta_it->second[getMetaID(meta_it,ur_id)][profile_values][HARD_MIN] != "-"){
            // Hard limit test
            // Hard limit min
            if (stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][getIndex(profile_values)],nullptr)  < stod(meta_it->second[getMetaID(meta_it,ur_id)][profile_values][HARD_MIN],nullptr) ){
                if (verbose >= 0){
                    cout << "VERBOSE: ALERT: Lower hard limit" << endl;
                }
    
                // Count exact error value 
                cout << "TATA: " << stod(meta_it->second[getMetaID(meta_it,ur_id)][profile_values][HARD_MIN],nullptr) << " " << stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][getIndex(profile_values)],nullptr) << endl;
                err = stod(meta_it->second[getMetaID(meta_it,ur_id)][profile_values][HARD_MIN],nullptr) - stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][getIndex(profile_values)],nullptr);
                // Alert alert messaga (caption) and add alert code
                alert_message = "The device " + to_string(*ur_id) + " exceeded the minimal hard limit by " + to_string(err) + "5";
                addAlert(profile_values, alert_message, alert_str);
            }
            //hard limit max
            if (stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][getIndex(profile_values)],nullptr)  > stod(meta_it->second[getMetaID(meta_it,ur_id)][profile_values][HARD_MAX],nullptr) ){
                if (verbose >= 0){
                    cout  << "VERBOSE: ALERT: Higher hard limit" << endl;
                }
                // Count exact error value 
                err = stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][getIndex(profile_values)],nullptr)  - stod(meta_it->second[getMetaID(meta_it,ur_id)][profile_values][HARD_MAX],nullptr);
                // Alert alert messaga (caption) and add alert code
                alert_message = "The device " + to_string(*ur_id) + " exceeded the maximal hard limit by " + to_string(err) + "6";
                addAlert(profile_values, alert_message, alert_str);
            }
        }
    }
}

void Analyzer::dataChangeCheck(map<uint64_t,vector<double> >::iterator &sensor_it ,map<string, map<uint64_t, map<string, vector<string> > > >::iterator &meta_it, string ur_field, uint64_t *ur_id, double *ur_time ,double *ur_data, map<string,vector<string> > &alert_str){
    double alert_coef = 0;
    double new_value = 0;
    double profile_value = 0;
    string alert_message;

    for (auto profile_values: meta_it->second[getMetaID(meta_it,ur_id)]["profile"]){
        new_value = stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][getIndex(profile_values)],nullptr);
        profile_value = stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaProfile"][getIndex(profile_values)],nullptr);
        // Protection against zero profile values
        if (profile_value == 0){
            profile_value = 1;
        }

        // Test if grow limits are set
        // Grow up,down limits are dependent -> test for grow up is ok
        if (meta_it->second[getMetaID(meta_it,ur_id)][profile_values][GROW_UP] != "-"){
            // Divide by zero protection
            if (stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][getIndex(profile_values)],nullptr) == 0 ){
                alert_coef = 1; //-> mean no data grow change
            } else {
                alert_coef = new_value / profile_value;
            }
            // Test grow limits
            if (alert_coef > stod(meta_it->second[getMetaID(meta_it,ur_id)][profile_values][GROW_UP],nullptr)){
                if (verbose >= 0){
                    cout << "VERBOSE: ALERT: GROW UP with value " << alert_coef << endl;
                }
                // Alert alert messaga (caption) and add alert code
                alert_message = "The device " + to_string(*ur_id) + " exceeded the grow up limit with value " + to_string(alert_coef) + "7"; 
                addAlert(profile_values, alert_message, alert_str);

            }
            if (alert_coef < stod(meta_it->second[getMetaID(meta_it,ur_id)][profile_values][GROW_DOWN],nullptr)){
                if (verbose >= 0){
                    cout << "VERBOSE: ALERT: GROW DOWN " << endl;
                }
                // Alert alert messaga (caption) and add alert code
                alert_message = "The device " + to_string(*ur_id) + " exceeded the grow down limit with value " + to_string(alert_coef) + "8";
                addAlert(profile_values, alert_message, alert_str);
            }
        }
    }
}

map<string,vector<string> > Analyzer::analyzeData(string ur_field, uint64_t *ur_id, double *ur_data, double *ur_time) {
    map<string, map<uint64_t, map<string, vector<string> > > >::iterator meta_it;
    map<uint64_t, vector<double> >::iterator sensor_it;
    map<string, vector<string> > alert_str;

    // Find proper iterators
    meta_it = series_meta_data.find(ur_field);
    sensor_it = control[ur_field].find(*ur_id);

    // Push new data and do calculation
    rotate( sensor_it->second.begin(), sensor_it->second.begin()+1, sensor_it->second.end());
    pushData(ur_time, ur_data, ur_id, meta_it, sensor_it, "metaData");
    modifyMetaData(ur_field,ur_id,meta_it, sensor_it, "metaData");

    /*
    * Check conditions
    */
    // Soft, hard limit check
    dataLimitCheck(meta_it, ur_field, ur_id, ur_time ,ur_data, alert_str);
    // Grow check
    dataChangeCheck(sensor_it, meta_it, ur_field, ur_id, ur_time, ur_data, alert_str);
    sensor_it = control[ur_field].find(*ur_id);

    // Do action - return result
    return alert_str;

}

/*
ALERT Codes
1 - Inactivity period 
2 - Static data limit 
3 - Soft limit min
4 - Soft limit max
5 - Hard limit min
6 - Hard limit max
7 - Grow Up limit
8 - Grow Down limit
*/
void Analyzer::sendAlert(map<string, vector<string> > &alert_str, string &ur_field, uint64_t *ur_id, double *ur_time){
    if (alert_str.empty()){
        if (verbose >= 1){
            cout << "VERBOSE: No alert detected" << endl;
        }
        return;
    }

    map<string, map<uint64_t, map<string, vector<string> > > >::iterator meta_it;
    meta_it = series_meta_data.find(ur_field);
    double err_value = 0;
    double profile_value = 0;
    int index = 0;
    string alert_code;

    for (auto profile: alert_str){
        for (auto elem: profile.second){
            if (verbose >= 1){
                cout << "VERBOSE: Send alert: " << profile.first << ", " << elem << endl;
            }
            index = getIndex(profile.first);
            if (index != -1){
                err_value = stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][index],nullptr);
                profile_value = stod(meta_it->second[getMetaID(meta_it,ur_id)]["metaProfile"][index],nullptr);
            } else {
                err_value = 0;
                profile_value = 0;
            }
            //Get alert code from CAPTION message
            alert_code = elem.substr(elem.size()-1,elem.size());
            elem = elem.substr(0, elem.size()-1);
            
            // Convert timestamp to UniRec time
            ur_time_t timestamp = ur_time_from_sec_msec(*ur_time,0);
            // Clear variable-length fields
            ur_clear_varlen(alert_template, data_alert);
            // Set UniRec message values
            ur_set(alert_template, data_alert, F_DEV_ADDR, *ur_id);
            ur_set(alert_template, data_alert, F_TIME, timestamp);
            ur_set(alert_template, data_alert, F_ALERT_CODE, stod(alert_code));
            ur_set(alert_template, data_alert, F_ERR_VALUE, err_value);
            ur_set(alert_template, data_alert, F_PROFILE_VALUE, profile_value);
            ur_set_string(alert_template, data_alert, F_PROFILE_KEY, profile.first.c_str());
            ur_set_string(alert_template, data_alert, F_CAPTION, elem.c_str());
            ur_set_string(alert_template, data_alert, F_UR_KEY, ur_field.c_str());
            trap_ctx_send(alert_ifc, 0, data_alert, ur_rec_size(alert_template, data_alert) );
            //trap_ctx_send_flush(alert_ifc,0);
        }
    }
}

void Analyzer::periodicCheck(int period,  string ur_field, uint64_t *ur_id){

    map<string, map<uint64_t, map<string, vector<string> > > >::iterator meta_it;
    meta_it = series_meta_data.find(ur_field);
    double change_period = 0;
    uint64_t data_id = *ur_id;
    double ur_time = 0;
    string alert_message;
    int err = 0;

    while(true){
        sleep(period);
        int result = std::time(nullptr);
        if (result - stoi(meta_it->second[getMetaID(meta_it,&data_id)]["metaData"][LAST_TIME]) > period ){
            map<string,vector<string> > alert_str;
            // Alert -> data hasn't been received
            if (verbose >= 0){
                cout << "VERBOSE: ALERT: inactivity data period exceeded" << endl;
            }

            // Count the exact error value
            err = (result - stoi(meta_it->second[getMetaID(meta_it,&data_id)]["metaData"][LAST_TIME]));
            // Alert alert messaga (caption) and add alert code
            alert_message = "The device " + to_string(*ur_id) + " exceeded the inactivity period by " + to_string(err) + "1";
            addAlert(ur_field, alert_message, alert_str);
            ur_time = stod(meta_it->second[getMetaID(meta_it,&data_id)]["metaData"][LAST_TIME],nullptr);
            sendAlert(alert_str, ur_field, &data_id, &ur_time);
        }

        // Check if periodic data change is enabled
        if ( meta_it->second[getMetaID(meta_it,&data_id)]["general"][PERIODIC_INTERVAL] != "-"){
            // Compare new and previous values
            if ( stod(meta_it->second[getMetaID(meta_it,&data_id)]["metaData"][NEW_ORIG_VALUE],nullptr) == stod(meta_it->second[getMetaID(meta_it,&data_id)]["metaData"][PREV_VALUE],nullptr) ){
                change_period = stod(meta_it->second[getMetaID(meta_it,&data_id)]["metaProfile"][CHANGE_PERIOD],nullptr);
                change_period++;
                meta_it->second[getMetaID(meta_it,&data_id)]["metaProfile"][CHANGE_PERIOD] = to_string(change_period);
            // In case of difference reset counter
            } else {
                  meta_it->second[getMetaID(meta_it,&data_id)]["metaProfile"][CHANGE_PERIOD] = to_string(0);
                  change_period = 0;
            }

            // If counter is higher than specified limit -> send an alert
            if (change_period > stod(meta_it->second[getMetaID(meta_it,&data_id)]["general"][PERIODIC_INTERVAL],nullptr)){
                map<string,vector<string> > alert_str;
                if (verbose >= 0){
                    cout << "VERBOSE: ALERT: data hasn't been chagend long time" << endl;
                }

                // Count the exact error value
                err = change_period - stod(meta_it->second[getMetaID(meta_it,&data_id)]["general"][PERIODIC_INTERVAL],nullptr);
                // Alert alert messaga (caption) and add alert code
                alert_message = "The device " + to_string(*ur_id) + " exceeded the static data limit by " + to_string(err) + "2";
                addAlert(ur_field, alert_message, alert_str);
                ur_time = stod(meta_it->second[getMetaID(meta_it,&data_id)]["metaData"][LAST_TIME],nullptr);
                sendAlert(alert_str, ur_field, &data_id, &ur_time);
            }

        }
    }
}

void Analyzer::periodicExport(int period, string ur_field, uint64_t *ur_id){

    map<string, map<uint64_t, map<string, vector<string> > > >::iterator meta_it;
    meta_it = series_meta_data.find(ur_field);
    uint64_t data_id = *ur_id;
    if (verbose >= 0){
        cout << "VERBOSE: Periodic export " << ur_field <<endl;
    }
    while (true){
        sleep (period);
        for (auto elem: ur_export_fields){
            for (auto field: elem.second){
                // Set unirec record
                if (field == "MOVING_AVERAGE"){
                    ur_set(export_template[elem.first], data_export[elem.first], F_MOVING_AVERAGE, stod(meta_it->second[getMetaID(meta_it,&data_id)]["metaData"][MOV_AVERAGE],nullptr));
                } else if (field == "MOVING_VARIANCE") {
                    ur_set(export_template[elem.first], data_export[elem.first], F_MOVING_VARIANCE, stod(meta_it->second[getMetaID(meta_it,&data_id)]["metaData"][MOV_VARIANCE],nullptr));

                } else if (field == "MOVING_MEDIAN"){
                    ur_set(export_template[elem.first], data_export[elem.first], F_MOVING_MEDIAN, stod(meta_it->second[getMetaID(meta_it,&data_id)]["metaData"][MOV_MEDIAN],nullptr));

                } else if (field == "AVERAGE"){
                    ur_set(export_template[elem.first], data_export[elem.first], F_AVERAGE, stod(meta_it->second[getMetaID(meta_it,&data_id)]["metaData"][AVERAGE],nullptr));
                }
            }
            // Send data for periodic export
            trap_ctx_send(export_ifc, elem.first, data_export[elem.first], ur_rec_size(export_template[elem.first], data_export[elem.first]));
        }
    }
}

void Analyzer::runThreads(string &ur_field, uint64_t *ur_id){
    map<string, map<uint64_t, map<string, vector<string> > > >::iterator meta_it;
    meta_it = series_meta_data.find(ur_field);
    // Run periodic check if it is set and skip repeated calls using flag
    if (meta_it->second[getMetaID(meta_it,ur_id)]["general"][PERIODIC_CHECK] != "-" && meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][CHECKED_FLAG] == "x"){
        thread t1(&Analyzer::periodicCheck,this,stoi(meta_it->second[getMetaID(meta_it,ur_id)]["general"][PERIODIC_CHECK],nullptr),ur_field, ur_id);
        t1.detach();
        meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][CHECKED_FLAG] = "p";
    }

    if (meta_it->second[getMetaID(meta_it,ur_id)]["general"][EXPORT_INTERVAL] != "-" && ( meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][CHECKED_FLAG] == "x" || meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][CHECKED_FLAG] == "p" )){
        thread t2(&Analyzer::periodicExport,this,stoi(meta_it->second[getMetaID(meta_it,ur_id)]["general"][EXPORT_INTERVAL],nullptr),ur_field, ur_id);
        t2.detach();
        meta_it->second[getMetaID(meta_it,ur_id)]["metaData"][CHECKED_FLAG] = "e";

    }
}

// Data series processing
void Analyzer::processSeries(string ur_field, uint64_t *ur_id, double *ur_time, double *ur_data) {
    int init_state = 0;
    // Initialize data series
    /*
    * return values:
    *  0 - init done
    *  1 - ur_field is not specified in the template
    *  2 - new record has been created
    *  3 - values were rotated
    *  4 - new item was added - learning phase
    *  5 - ignore phase
    *  6 - record ID is not configured
    */
    init_state = initSeries(ur_field, ur_id, ur_data, ur_time);

    if (init_state == 0){
        // Analyze data series
        if (verbose >= 0){
            cout << "VERBOSE: Analyzing phase" << endl;
        }
        auto alert_str = analyzeData(ur_field, ur_id, ur_data, ur_time);
        sendAlert(alert_str, ur_field, ur_id, ur_time);
        runThreads(ur_field, ur_id);
    }

    if (init_state != 1 && init_state != 6 && verbose >= 0){
        printSeries(ur_field,ur_id);
    }
}
/*
 * END TIME SERIES PROCESS
 */
