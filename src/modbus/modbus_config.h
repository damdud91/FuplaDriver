#ifndef MODBUS_CONFIG_H_
#define MODBUS_CONFIG_H_
//########################################
#include <stdint.h>
#include <stdbool.h>
#include "stm32f302xc.h"
#include "modbus.h"
#include "main.h"

//Domyslny adres modbus
#define MODBUS_ADDR 10

//Predkosci transmisji po RS
#define MODBUS_2400 15000
#define MODBUS_9600 3750
#define MODBUS_19200 1875
#define MODBUS_38400 937
#define MODBUS_57600 625
#define MODBUS_115200 312

//Makra dla biblioteki
#define USART_RXD_ISR_COMPLETE USART2->ISR & USART_ISR_RXNE
#define USART_RXD_REGISTER USART2->RDR

#define USART_TXD_REGISTER USART2->TDR
#define USART_TXD_ISR_CLEAR USART2->ICR |= USART_ICR_TCCF

#define USART_SETTLING_ISR_COMPLETE USART2->ISR & USART_ISR_RTOF
#define USART_SETTLING_ISR_CLEAR USART2->ICR |= USART_ICR_RTOCF

#define CRC_RESET CRC->CR |= CRC_CR_RESET
#define CRC_CALC(x) *((uint8_t *)&CRC->DR) = (uint8_t)x
#define CRC_GET_LOW (uint8_t)(CRC->DR & 0xFF)
#define CRC_GET_HIGH (uint8_t)(CRC->DR>>8 & 0xFF)

//########################################
void MODBUS_Init(void);

//########################################
#endif /* MODBUS_CONFIG_H_ */
