/*
 * incremental_encoder.h
 *
 *  Created on: 05.09.2019
 *      Author: damian
 */

#ifndef INCREMENTAL_ENCODER_H_
#define INCREMENTAL_ENCODER_H_

#include "stm32f302xc.h"
#include "main.h"

#define ENCODER_MAX_VAL 262143
#define ENCODER_VAL ((uint32_t)(encoder_val_h<<16|TIM1->CNT))

extern volatile uint16_t encoder_val_h;

void ENCODER_Init(void);

#endif /* INCREMENTAL_ENCODER_H_ */
