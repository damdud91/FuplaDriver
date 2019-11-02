/*
 * debug.c
 *
 *  Created on: 21 paź 2019
 *      Author: dudek
 */
#include "debug.h"

volatile char debug_buff[128];
volatile char* debug_head=debug_buff;
volatile char* debug_tail=debug_buff;
bool debug_mute;

static void DEBUG_buff_append(char*);
static void DEBUG_start(void);

//########################################
void DEBUG_Init(void)
{
#if DEBUG_ONOFF
    //GPIO B6-TXD, B7-RXD
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN; //Wl. zasilania
    GPIOB->AFR[0] |= 0x7<<24 | 0x7<<28; //Funkcje alternatywne dla pinow
    GPIOB->MODER |= GPIO_MODER_MODER6_1|GPIO_MODER_MODER7_1; //Funkcje alternatywne
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR6|GPIO_OSPEEDER_OSPEEDR7; //High speed
    GPIOB->PUPDR |= GPIO_PUPDR_PUPDR7_0; //Pull up

    //USART 1
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN; //Wl. zasilania modulu USART1
    USART1->BRR = 0x271; //Baud rate 115200bps
    USART1->ICR |= USART_ICR_TCCF; //Zerowanie flagi
    NVIC->ISER[1] |= 1<<5;//Wl. przerwania USART1
    USART1->CR1 = USART_CR1_RE|USART_CR1_TE|USART_CR1_TCIE|USART_CR1_UE; //Enable USART2
#endif
}
void DEBUG_Handler(void)
{
	USART1->ICR |= USART_ICR_TCCF;
	if(debug_tail!=debug_head)
	{
		USART1->TDR=*debug_tail++;
		if(debug_tail==debug_buff+128) debug_tail=debug_buff;
	}
}
static void DEBUG_buff_append(char* text)
{
	if(!debug_mute)
	{
		*debug_head++=*text++;
		if(debug_head==debug_buff+128) debug_head=debug_buff;
		if(debug_head==debug_tail)
		{
			debug_head--;
			if(debug_head<debug_buff) debug_head=debug_buff+127;
		}
	}
}
static void DEBUG_start(void)
{
	if(!debug_mute && debug_tail!=debug_head)
	{
		USART1->TDR=*debug_tail++;
		if(debug_tail==debug_buff+128) debug_tail=debug_buff;
	}
}

//########################################
void DEBUG_print_text(char* text)
{
#if DEBUG_ONOFF
	while(*text!='\0') DEBUG_buff_append(text++);
	DEBUG_start();
#endif
}
void DEBUG_print_int(int32_t value)
{
#if DEBUG_ONOFF
	uint8_t vals[]={0,0,0,0,0,0,0,0,0,0},i,cnt=0;
	uint32_t tmp;
	char a;

	if(value<0)
	{
		DEBUG_buff_append("-");
		value = (0xFFFFFFFF^value)+1;
	}
	while(value!=0)
	{
		tmp = (uint32_t)pow(10,cnt+1);
		vals[cnt] = (value%tmp)/(tmp/10);
		value -= vals[cnt++]*(tmp/10);
	}
	for(i=1;i<=cnt;i++)
	{
		a = (char)(vals[cnt-i]+'0');
		DEBUG_buff_append(&a);
	}
	DEBUG_start();
#endif
}
void DEBUG_print_var(char* name, int32_t value)
{
#if DEBUG_ONOFF
	DEBUG_print_text(name);
	DEBUG_print_text(" = ");
	DEBUG_print_int(value);
	DEBUG_print_text("\n");
#endif
}
