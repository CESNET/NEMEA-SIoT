/**
 * \file black_list.c
 * \brief LoRaWAN Detector of NEMEA module.
 * \author Erik Gresak <erik.gresak@vsb.cz>
 * \date 2018
 */
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
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include "black_list.h"

/** Define structure for BlackList */
struct bl_device {
    uint64_t DEV_ADDR;
    double AIR_TIME;
    uint8_t ENABLE;
    uint64_t TIMESTAMP;
    uint16_t LAST_FCNT;
    uint8_t RESTART;
    struct bl_device *next;
};

struct bl_device *head = NULL;
struct bl_device *current = NULL;

/** 
 * BlackList
 * Contain sensors list with identification field DevAddr. Blocking 
 * list that allow block messages from LoRaWAN infrastructure that 
 * have a information of history and actual air-time.
 */

void bl_insert_device(uint64_t dev_addr, uint64_t timestamp, double air_time, uint8_t enable) {

    struct bl_device *add = (struct bl_device*) malloc(sizeof (struct bl_device));

    add->DEV_ADDR = dev_addr;
    add->AIR_TIME = air_time;
    add->TIMESTAMP = timestamp;
    add->ENABLE = enable;
    add->next = head;
    add->RESTART = 0;
    head = add;
}

uint8_t bl_is_empty() {
    head = NULL;
    return 0;
}

uint8_t bl_is_exist(uint64_t dev_addr) {
    if (bl_get_device(dev_addr) != NULL)
        return 1;
    return 0;
}

struct bl_device* bl_get_device(uint64_t dev_addr) {

    struct bl_device* current = head;

    if (head == NULL)
        return NULL;

    while (current->DEV_ADDR != dev_addr) {
        if (current->next == NULL)
            return NULL;
        else
            current = current->next;
    }

    return current;
}