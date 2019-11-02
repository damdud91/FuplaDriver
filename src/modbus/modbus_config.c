/*
 * "Interfejs MODBUS dla STM32"
 * Author: Damian Dudek
 * Date: 01.09.2019
 */
#include "modbus_config.h"

//######################################## Inicjalizacja
void MODBUS_Init(void)
{
    uint8_t tmp;

    //GPIO A2-TXD, A3-RXD, A1-DE
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; //Wl. zasilania
    GPIOA->AFR[0] |= 0x7<<4 | 0x7<<8 | 0x7<<12; //Funkcje alternatywne dla pinow
    GPIOA->MODER |= GPIO_MODER_MODER1_1|GPIO_MODER_MODER2_1|GPIO_MODER_MODER3_1; //Funkcje alternatywne
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR1|GPIO_OSPEEDER_OSPEEDR2|GPIO_OSPEEDER_OSPEEDR3; //High speed

    //USART 2
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN; //Wl. zasilania modulu USART2
    USART2->BRR = modbus_settings.baudrate; //Baud rate
    USART2->CR3 |= USART_CR3_DEM; //Wl. obslugi pinu DE
    USART2->CR2 |= USART_CR2_RTOEN; //Wl. obslugi settling
    USART2->RTOR = 10; //settling time
    tmp=USART_RXD_REGISTER;
    USART2->ICR |= USART_ICR_TCCF; //Zerowanie flagi
    NVIC->ISER[1] |= 1<<6;//Wl. przerwania USART2
    USART2->CR1 = USART_CR1_RXNEIE|USART_CR1_RTOIE|USART_CR1_RE|USART_CR1_TCIE|USART_CR1_TE|USART_CR1_DEAT_0|USART_CR1_DEDT_0|USART_CR1_UE;//Enable USART2

    //CRC
    RCC->AHBENR |= RCC_AHBENR_CRCEN; //Wl. zasilania modulu CRC
    CRC->INIT = 0xFFFF; //wartosc poczatkowa
    CRC->CR |= CRC_CR_POLYSIZE_0|CRC_CR_REV_IN_0|CRC_CR_REV_OUT; //16bit polynomial, input reverse byte, output reverse enable
    CRC->POL = 0x8005; //wartoisc polynomial
    CRC->CR |= CRC_CR_RESET; //Reset answer
}
