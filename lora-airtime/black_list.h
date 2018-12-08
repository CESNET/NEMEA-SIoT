/* 
 * File:   black_list.h
 * Author: Erik Gresak
 * Email: gre0071@vsb.cz
 * Created on March 21, 2018, 9:12 AM
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#ifndef BLACK_LIST_H
#define BLACK_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

    void bl_insert_device(char* dev_addr, uint64_t time_stamp, double air_time, uint8_t enable);
    struct bl_device* bl_get_device(char* dev_addr);
    uint8_t bl_is_empty();
    uint8_t bl_compare(char* a, char* b, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* BLACK_LIST_H */
