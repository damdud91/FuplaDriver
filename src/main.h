#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f302xc.h"
#include "core_cm4.h"

#include "modbus.h"
#include "modbus_config.h"
#include "eeprom.h"
#include "incremental_encoder.h"
//#include "ams.h"
#include "motor.h"
#include "pid.h"
#include "modbus_table.h"
#include "debug.h"

//EEPROM
#define EEPROM_NB_OF_VAR 8

//IO
#define INPUTS_CH1 ((GPIOB->IDR & (1<<15))>>15)
#define INPUTS_CH2 ((GPIOC->IDR & (1<<15))>>15)
#define OUTPUTS_CH1(x) TIM4->CCR4 = x
#define OUTPUTS_CH2(x) TIM4->CCR3 = x

//SYSTEM
extern volatile uint16_t tick_1ms;
extern volatile uint16_t tick_10ms;
extern volatile uint16_t tick_100ms;

#define STATUS_STARTED 0
#define STATUS_DEBUG 1
#define STATUS_MUTE 2
#define STATUS_RESET 3
#define STATUS_IN1 4
#define STATUS_IN2 5
#define STATUS_VOLTAGE_LIMIT 6
#define STATUS_CURRENT_LIMIT 7
#define STATUS_MODBUS_ERROR 8
#define STATUS_SYSTEM_ERROR 9

typedef struct{
	uint16_t status;
    uint16_t time;
    uint16_t set;
    uint16_t encoder_val;
    uint8_t outputs_set[2];

    uint8_t mode;
    uint16_t max_pos;
    uint16_t min_pos;
    uint16_t max_vel;
    uint16_t max_curr;
    uint16_t min_volatge;
} state_t;

extern volatile state_t sys_state;

#endif /* MAIN_H_ */
