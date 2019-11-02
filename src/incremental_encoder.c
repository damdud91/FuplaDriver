/*
 * incremental_encoder.c
 *
 *  Created on: 05.09.2019
 *      Author: damian
 */
#include "incremental_encoder.h"

volatile uint16_t encoder_val_h;

void ENCODER_Init(void)
{
    //PA8-A, PA9-B
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; //Wl. zasilania
    GPIOA->AFR[1] |= 0x6<<0 | 0x6<<4; //Funkcje alternatywne dla pinow
    GPIOA->MODER |= GPIO_MODER_MODER8_1|GPIO_MODER_MODER9_1; //Funkcje alternatywne
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8|GPIO_OSPEEDER_OSPEEDR9; //High speed
//    GPIOA->PUPDR |= GPIO_PUPDR_PUPDR8_0|GPIO_PUPDR_PUPDR9_0; //Pull up

    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; //Wl. modulu timera
    TIM1->CCMR1 |= TIM_CCMR1_CC1S_0|TIM_CCMR1_CC2S_0; //mapowanie wejsc na sygnaly
    TIM1->SMCR |= TIM_SMCR_SMS_0; //|TIM_SMCR_SMS_1; //tryb enkodera
    TIM1->ARR = 0xFFFF; //liczba impulsow na obrot
//    TIM1->CCER |= TIM_CCER_CC1E|TIM_CCER_CC2E;
//    TIM1->CCMR1 |= TIM_CCMR1_IC1PSC|TIM_CCMR1_IC2PSC; //prescaller = 4
    TIM1->CNT = ENCODER_MAX_VAL/100;
    TIM1->CR1 |= TIM_CR1_CEN; //wl licznika

    TIM1->DIER |= TIM_DIER_UIE;
    NVIC->ISER[0] |= 1<<25;//Wl. przerwania TIM1
}


