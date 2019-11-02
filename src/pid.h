/*
 * pid.h
 *
 *  Created on: 05.09.2019
 *      Author: damian
 */
#ifndef PID_H_
#define PID_H_

#include "../drivers/stm32f302xc.h"
#include "incremental_encoder.h"
#include "main.h"

int32_t Calc(uint32_t zadana,uint32_t zmierzona);

#endif /* PID_H_ */
