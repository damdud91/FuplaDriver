/*
 * debug.h
 *
 *  Created on: 21 paź 2019
 *      Author: dudek
 */

#ifndef MODBUS_DEBUG_H_
#define MODBUS_DEBUG_H_
//########################################
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "stm32f302xc.h"
#include "core_cm4.h"

#define DEBUG_ONOFF 1
extern bool debug_mute;

void DEBUG_Init(void);
void DEBUG_print_text(char*);
void DEBUG_print_int(int32_t);
void DEBUG_print_var(char*, int32_t);

void DEBUG_Handler(void);

//########################################
#endif /* MODBUS_DEBUG_H_ */
