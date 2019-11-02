/*
 * it.c
 *
 *  Created on: 07.09.2019
 *      Author: damian
 */
#include "interruption.h"

void SysTick_Handler(void)
{
    static uint8_t tmr=100;
    tmr--;

    tick_1ms++;
    if(!(tmr%10)) tick_10ms++;
    if(!tmr){ tick_100ms++; tmr=100;}
}

void USART1_IRQHandler(void)
{
    DEBUG_Handler();
}

void USART2_IRQHandler(void)
{
    MODBUS_Handler();
}

void SPI1_IRQHandler(void)
{
    uint16_t tmp;
    tmp = SPI1->DR;
}
void TIM1_UP_TIM16_IRQHandler(void)
{
	if(TIM1->CNT<0x3FFF) encoder_val_h++;
	else encoder_val_h--;
	TIM1->SR &= ~(TIM_SR_UIF);
}
