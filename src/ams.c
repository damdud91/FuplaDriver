/*
 * ams.c
 *
 *  Created on: 03.09.2019
 *      Author: damian
 */
#include "ams.h"

void AMS_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; //Wl. zasilania
    GPIOA->AFR[0] |= 0x5<<20 | 0x5<<24 | 0x5<<28; //Funkcje alternatywne dla pinow
    GPIOA->MODER |= GPIO_MODER_MODER5_1|GPIO_MODER_MODER6_1|GPIO_MODER_MODER7_1; //Funkcje alternatywne
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5|GPIO_OSPEEDER_OSPEEDR6|GPIO_OSPEEDER_OSPEEDR7; //High speed

    GPIOA->MODER |= GPIO_MODER_MODER4_0; //Wyprowadzenia jako wyjscia
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR4; //High speed
    GPIOA->BSRRH |= GPIO_BSRR_BS_4;

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; //Wl. modulu SPI
//    SPI1->CR1 = SPI_CR1_SSI|SPI_CR1_SSM|SPI_CR1_DFF|SPI_CR1_BR|SPI_CR1_MSTR|SPI_CR1_CPHA;
//    SPI1->CR2 |= SPI_CR2_RXNEIE;
//    NVIC->ISER[1] |= 1<<3;//Wl. przerwania USART2
    SPI1->CR1 |= SPI_CR1_SPE;
}
void AMS_Read(void)
{
    uint16_t tmp,tmp2;

    SPI1->SR &= ~SPI_SR_RXNE;
    SPI1->DR = 0x7FFE;
    while(!(SPI1->SR & SPI_SR_RXNE)){}
    GPIOA->BSRRL |= GPIO_BSRR_BS_4;
    GPIOA->BSRRH |= GPIO_BSRR_BS_4;

    tmp = SPI1->DR;
    SPI1->DR = 0xFFFF;
    while(!(SPI1->SR & SPI_SR_RXNE)){}
    GPIOA->BSRRL |= GPIO_BSRR_BS_4;
    GPIOA->BSRRH |= GPIO_BSRR_BS_4;

    tmp = SPI1->DR;
    SPI1->DR = 0xC000;
    while(!(SPI1->SR & SPI_SR_RXNE)){}
    GPIOA->BSRRL |= GPIO_BSRR_BS_4;
    GPIOA->BSRRH |= GPIO_BSRR_BS_4;

    tmp2 = SPI1->DR;

}

