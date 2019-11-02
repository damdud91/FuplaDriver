/*
 * motor.c
 *
 *  Created on: 05.09.2019
 *      Author: damian
 */
#include "motor.h"

void MOTOR_Init(void)
{
    //PB10-PWM
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN; //Wl. zasilania
    GPIOB->AFR[1] |= 0x1<<8; //Funkcje alternatywne dla pinow
    GPIOB->MODER |= GPIO_MODER_MODER10_1; //Funkcje alternatywne
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR10; //High speed

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //Wl. modulu timera
    TIM2->CR1 |= TIM_CR1_ARPE|TIM_CR1_CMS; //buforowanie ARR, center aligned
    TIM2->CCMR2 |= TIM_CCMR2_OC3M|TIM_CCMR2_OC3PE; //tryb PWM
    TIM2->CCER |= TIM_CCER_CC3E|TIM_CCER_CC3P; //wl wyjscia PWM z licznika
    TIM2->ARR = 0x7FFF; //wartosc graniczna pwm
    TIM2->EGR |= TIM_EGR_UG; //aktualizuj rejestry
    TIM2->CR1 |= TIM_CR1_CEN; //wl licznika
    MOTOR_SET_PWM(0);

    //PB0-IN1 PB12-IN2
    GPIOB->MODER |= GPIO_MODER_MODER0_0|GPIO_MODER_MODER12_0; //PB0;PB12 jako wyjscie
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0|GPIO_OSPEEDER_OSPEEDR12; //High speed
    GPIOB->BSRRH |= GPIO_BSRR_BS_0|GPIO_BSRR_BS_12; //stan niski

}

void MOTOR_Move(int32_t x)
{
    if(x>20){MOTOR_RIGHT((uint16_t)x);}
    else if(x<-20){MOTOR_LEFT((uint16_t)-x);}
    else {MOTOR_STOP();}
}


