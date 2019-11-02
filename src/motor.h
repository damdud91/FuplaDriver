/*
 * motor.h
 *
 *  Created on: 05.09.2019
 *      Author: damian
 */
#ifndef MODBUS_MOTOR_H_
#define MODBUS_MOTOR_H_

#include "stm32f302xc.h"

#define MOTOR_GET_PWM() TIM2->CCR3 //wartosc PWM
#define MOTOR_SET_PWM(x) TIM2->CCR3 = x //wartosc PWM
#define MOTOR_L GPIOB->BSRRH |= GPIO_BSRR_BS_0; GPIOB->BSRRL |= GPIO_BSRR_BS_12 //left
#define MOTOR_R GPIOB->BSRRL |= GPIO_BSRR_BS_0; GPIOB->BSRRH |= GPIO_BSRR_BS_12 //right
#define MOTOR_S GPIOB->BSRRL |= GPIO_BSRR_BS_0; GPIOB->BSRRL |= GPIO_BSRR_BS_12 //right

#define MOTOR_LEFT(x) MOTOR_L;MOTOR_SET_PWM(x)
#define MOTOR_RIGHT(x) MOTOR_R;MOTOR_SET_PWM(x)
#define MOTOR_STOP() MOTOR_S;MOTOR_SET_PWM(0x0)

void MOTOR_Init(void);
void MOTOR_Move(int32_t x);


#endif /* MODBUS_MOTOR_H_ */
