/*
 * it.h
 *
 *  Created on: 07.09.2019
 *      Author: damian
 */
#ifndef INTERRUPTION_H_
#define INTERRUPTION_H_

#include "stm32f302xc.h"
#include "core_cm4.h"
#include "main.h"

void SysTick_Handler(void);
void USART2_IRQHandler(void);
void SPI1_IRQHandler(void);

#endif /* INTERRUPTION_H_ */
