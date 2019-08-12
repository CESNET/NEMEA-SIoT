/**
 * \file DataDetector.cpp
 * \brief Receive, process and analyze incoming unirec data based on configuration. Send result to the output.
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

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <getopt.h>
#include <signal.h>
#include <thread>
#include <unistd.h>

#include <libtrap/trap.h>
#include <unirec/unirec.h>
#include "fields.h"
#include "ConfigParser.h"
#include "config.h"
#include "Analyzer.h"

using namespace std;

trap_module_info_t *module_info = NULL;

#define MODULE_BASIC_INFO(BASIC) \
  BASIC("data-series-detector", "This module detect anomalies in data series", 1, 1)
#define MODULE_PARAMS(PARAM) \
  PARAM('c', "config", "Configuration files with detection rules", required_argument, "string")

/*
* Print configured data from configuration file
* \param[in] series_meta_data Stucture with configured data
*
* NOTE: Auto specifier in function parameter is available from c++14 -> author used C++11
*/
void printSeries( map<string,map<uint64_t, map<string, vector<string> > > >& series_meta_data){
    cout << "printSeries method" << endl;
    for (auto main_key: series_meta_data){
        cout << "main key: " << main_key.first << endl;
        for (auto ids: series_meta_data[main_key.first]){
            cout << "main id: " << ids.first << endl;
            for (auto element : series_meta_data[main_key.first][ids.first]) {
                cout << " key: " << element.first << endl;
                cout << "  values: " << endl;
                for (auto elem: element.second){
                    cout << "   " << elem << endl;
                }
            }
        }
    }
}

/*
* Prepare output interfaces for periodic export
*
* \param[in] series_meta_data
* \param[out] export_template Unirec templates for export interfaces
* \param[out] ctx_export Export trap interfaces
* \param[out] data_export Unirec allocated records for export interfaces
* \param[out] ur_export_fields Map of unirec keys for each interface
* \param[in] verbose Verbose level
* \returns Result of initialization. 0 and 1 is success. Other values are errors.
*/
int initExportInterfaces(map<string, map<uint64_t, map<string, vector<string> > > > &series_meta_data,  ur_template_t *** export_template, trap_ctx_t **ctx_export, void ***data_export, map<int, vector<string> > &ur_export_fields, int verbose){
    string interface_spec;      // Name of output interface
    int flag = 0;               // Flag for definig output interface name
    vector<string> field_name;  // Tmp value for export ur_values
    int number_of_keys = 0;     // Counter for number of export values
    string tmp_ur_export;       // Unirect export format

    // Go through the configuration data
    for (auto main_key: series_meta_data){
        for (auto ids : series_meta_data[main_key.first]) {
            for (auto element: series_meta_data[main_key.first][ids.first]){
                // Find the export key in configuration data
                if (element.first == "export"){
                    // Begin initialization -> clear old tmp variables
                    flag = 0;
                    field_name.clear();
                    for (auto elem: element.second){
                        // Skip empty values
                        if(elem == "-"){
                            break;
                        }
                        // Update tmp variables
                        field_name.push_back(elem);
                        // First item found -> create export interface record
                        if(flag == 0){
                            interface_spec += "u:export-"+main_key.first+to_string(ids.first)+",";
                            number_of_keys++;
                            flag = 1;
                            if (verbose >= 0 ){
                                cout << "VERBOSE: Creating export interface: u:export-" << main_key.first+to_string(ids.first) << endl;
                            }
                        }
                    }
                    // Insert tmp variables to the map structure
                    if (flag == 1){
                        ur_export_fields.insert(pair<int, vector<string> >(number_of_keys-1, field_name));
                    }
                }
            }
        }
    }


    // No export parameters were specified -> no initialization required
    if (interface_spec.length() == 0){
        return 1;
    }
    // Remove the last comma
    interface_spec.pop_back();

    // Allocate memory for output export interfaces
    *export_template = (ur_template_t **)calloc(number_of_keys,sizeof(*export_template));
    *data_export = (void **) calloc(number_of_keys,sizeof(void *));
    if (export_template == NULL){
        cerr << "ERROR: Export output template allocation error" << endl;
        return 2;
    }
    if (*data_export == NULL){
        cerr << "ERROR: Export output data record allocation error" << endl;
        return 2;
    }

    // Interface initialization
    *ctx_export = trap_ctx_init3("data-periodic-export", "Export data profile periodicaly",0,number_of_keys,interface_spec.c_str(),NULL);
    if (*ctx_export == NULL){
        cerr << "ERROR: Data export interface initialization failed" << endl;
        return 3;
    }

    if (trap_ctx_get_last_error(ctx_export) != TRAP_E_OK){
        cerr << "ERROR in TRAP initialization: " << trap_ctx_get_last_error_msg(ctx_export) << endl;
        return 3;
    }

    // Interface control setting & create unirec template
    for (int i = 0; i < number_of_keys; i++ ){
        if ( trap_ctx_ifcctl(*ctx_export, TRAPIFC_OUTPUT,i,TRAPCTL_SETTIMEOUT,TRAP_WAIT) != TRAP_E_OK ) {
            cerr << "ERROR: export interface control setup failed" << endl;
            return 4;
        }

        // Clear old value in tmp variable
        tmp_ur_export.clear();
        // Create unirec export format
        for (auto elem: ur_export_fields[i]){
            tmp_ur_export += elem + ",";
        }
        // Remove the last comma
        tmp_ur_export.pop_back();

        *(export_template)[i] = ur_ctx_create_output_template(*ctx_export,i,tmp_ur_export.c_str(),NULL);
        if ( (*export_template)[i] == NULL ) {
            cerr << "ERROR: Unable to define unirec fields" << endl;
            return 5;
        }

        // Create record with no variable lenght memory
        (*data_export)[i] = ur_create_record((*export_template)[i], 0);
        if ( (*data_export)[i] == NULL ) {
            cerr << "Error: Data are not prepared for the export template" << endl;
            return 6;
        }
    }
    return 0;
}

/*
* Main function
*/
int main (int argc, char** argv){

    int exit_value = 0;                                       // Detector return value
    ur_template_t * in_template = NULL;                       // Unirec input template
    ur_template_t * alert_template = NULL;                    // Unirec output alert template
    ur_template_t **export_template = NULL;                   // Unirec output export template
    trap_ctx_t *ctx = NULL;                                   // Trap interfaces for incoming and lert data
    trap_ctx_t *ctx_export = NULL;                            // Trap interfaces for periodic export
    int verbose = 0;                                          // Verbose level
    void *data_alert = NULL;                                  // Unirec output alert record
    void **data_export = NULL;                                // Unirec output export record
    map<int, vector<string> > ur_export_fields; // Map with unirec values for each interface. The first key is number of interface and the second is name of record according to the configuration file (ur_field). In the last vector are profile items for export.

    int ret = 2;                                              // Tmp store variable
    uint64_t ur_id = 0;                                      // Tmp store variable
    double ur_time = 0;                                      // Tmp store variable
    double ur_data = 0;                                      // Tmp store variable
    string config_file = "";                                  // Configuration file

    /*
    ** interface initialization **
    */
    // Allocate and initialize module_info structure and all its members
    INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
    // Trap parameters processing
    trap_ifc_spec_t ifc_spec;
    ret = trap_parse_params(&argc, argv, &ifc_spec);
    if (ret != TRAP_E_OK) {
        if (ret == TRAP_E_HELP) { // "-h" was found
            trap_print_help(module_info);
            return 0;
        }
        cerr << "ERROR in parsing of parameters for TRAP: " << trap_last_error_msg << endl;
        return 1;
    }

    // Parse remaining parameters and get the configuration -> No additional param needed
    signed char opt;
    while ((opt = TRAP_GETOPT(argc, argv, module_getopt_string, long_options)) != -1) {
        switch (opt) {
        // Configuration file
        case 'c':
            config_file = optarg;
            break;
        default:
            cerr << "Error: Invalid arguments." << endl;
            FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
            return 1;
        }
    }

    // Parse created configuration file
    ConfigParser cp(config_file, verbose);
    cp.parseFile();
    auto series_meta_data = cp.getSeries();
    //printSeries(series_meta_data);
    if (series_meta_data.empty()){
        cerr << "ERROR configuration file is empty" << endl;
        return 1;
    }

    verbose = trap_get_verbose_level();
    if (verbose >= 0) {
        cout << "Verbosity level: " <<  trap_get_verbose_level() << endl;;
    }

    // Create analyze object
    Analyzer series_a (series_meta_data, verbose);

    // Check number of interfaces parameter
    if (strlen(ifc_spec.types) != 2) {
        cerr <<  "Error: Module requires just one input and one output interface" << endl;
        FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
        return 4;
    }

    if (verbose >= 0) {
        cout << "VERBOSE: Initializing TRAP library ..." << endl;
    }

    ctx = trap_ctx_init(module_info, ifc_spec);

    if (ctx == NULL) {
        cerr << "ERROR in TRAP initialization: " << trap_last_error_msg << endl;
        exit_value = 1;
        goto cleanup;
    }

    if (trap_ctx_get_last_error(ctx) != TRAP_E_OK){
      cerr << "ERROR in TRAP initialization: " << trap_ctx_get_last_error_msg(ctx) << endl;
      exit_value = 1;
      goto cleanup;
   }

    // Input interface control settings
    if (trap_ctx_ifcctl(ctx, TRAPIFC_INPUT, 0, TRAPCTL_SETTIMEOUT, TRAP_WAIT) != TRAP_E_OK) {
        cerr << "ERROR in input interface initialization" << endl;
        exit_value = 2;
        goto cleanup;
    }
    // Output interface control settings
    if (trap_ctx_ifcctl(ctx, TRAPIFC_OUTPUT,0,TRAPCTL_SETTIMEOUT, TRAP_WAIT) != TRAP_E_OK){
        cerr << "ERROR in alert output interface initialization" << endl;
        exit_value = 2;
        goto cleanup;
    }
    // Create empty input template
    in_template = ur_ctx_create_input_template(ctx, 0, NULL, NULL);
    if (in_template == NULL) {
        cerr <<  "ERROR: unirec input template create fail" << endl;
        exit_value = 2;
        goto cleanup;
    }
    // Create alert template
    alert_template = ur_ctx_create_output_template(ctx, 0, "ID,TIME,ur_key,alert_desc,profile_key,err_value,profile_value", NULL);
    if (alert_template == NULL) {
        cerr <<  "ERROR: unirec alert template create fail" << endl;
        exit_value = 2;
        goto cleanup;
    }

    // Initialize export output interfaces
    ret = initExportInterfaces(series_meta_data, &export_template, &ctx_export, &data_export, ur_export_fields,verbose);
    if (ret > 1){
        exit_value = 2;
        goto cleanup;
    }

    // Create alert record with maximum size of variable memory length
    data_alert = ur_create_record(alert_template, UR_MAX_SIZE);
        if ( data_alert == NULL ) {
            cout << "ERROR: Data are not prepared for alert template" << endl;
            exit_value = 3;
            goto cleanup;
        }

    if (verbose >= 0) {
        cout << "VERBOSE: Initialization done" << endl;
    }

    // Set initialized values to Analyze class
    series_a.setAlertInterface(ctx,alert_template,data_alert);
    series_a.setExportInterface(ctx_export, export_template, data_export, ur_export_fields);

    // Main loop for processing incoming data
    while (true){

        uint16_t memory_received = 0;
        const void *data_nemea_input = NULL;

        // Receive data and iterate over all fields
        TRAP_CTX_RECEIVE(ctx,0,data_nemea_input,memory_received,in_template);

        // Take ID and TIME field -> user for alert identification
        ur_id = *(ur_get_ptr(in_template, data_nemea_input, F_ID));
        ur_time = *(ur_get_ptr(in_template, data_nemea_input, F_TIME));

        // Go through all unirec fields
        ur_field_id_t id = UR_ITER_BEGIN;
        while ((id = ur_iter_fields(in_template, id)) != UR_ITER_END) {
            // Skip id -> not analyzed just used for alert identification
            if ( strcmp("ID",(ur_get_name(id))) == 0 ){
                continue;
            }
            // EOF close this module 
            if ( memory_received <= 1 ){
                char dummy[1] = {0};
                trap_ctx_send(ctx, 0, dummy, 1);
                trap_ctx_send_flush(ctx,0);
                goto cleanup;
            }

            if (verbose >= 2){
                cout << "VERBOSE: Received UniRec message with the record name" << ur_id << endl;
            }
            ur_data = *((double*) ur_get_ptr_by_id(in_template, data_nemea_input,id));
            // Analyze received data
            series_a.processSeries(ur_get_name(id), &ur_id, &ur_time, &ur_data);
        }
    }

cleanup:
    if (verbose >= 0){
        cout << "VERBOSE: Cleaning allocated structures" << endl;
    }
    // Clean alocated structures
    trap_ctx_finalize(&ctx);
    trap_ctx_finalize(&ctx_export);
    return exit_value;

}
