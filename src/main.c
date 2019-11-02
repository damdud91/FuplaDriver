#include "main.h"

//SYSTEM
volatile uint16_t tick_1ms;
volatile uint16_t tick_10ms;
volatile uint16_t tick_100ms;
volatile state_t sys_state;
void System_Init(void);

//ADC

//LED
#define LED_USR_OFF GPIOB->BSRRL |= GPIO_BSRR_BS_14
#define LED_USR_ON GPIOB->BSRRH |= GPIO_BSRR_BS_14
#define LED_USR(x) (x)?(LED_USR_ON):(LED_USR_OFF)
void LEDs_Init(void);
void LedTask(void);

//SWITCH
#define SWITCH_CH1 ((GPIOC->IDR & (1<<13))>>13)
#define SWITCH_CH2 ((GPIOC->IDR & (1<<14))>>14)
void Switch_Init(void);
void SwitchTask(void);

//IO
void IO_Init(void);

//MODBUS
void ModbusTableTask(void);
bool ModbusTable_3_space(void);
bool ModbusTable_2_space(void);
bool ModbusTable_1_space(void);
bool ModbusTable_0_space(void);

//EEPROM
#define EEPROM_VALID_MARKER 0xBABA
#define EEPROM_BASE_ADDR 0x1
#define EEPROM_CHECK_ADDR EEPROM_BASE_ADDR //1
#define EEPROM_MODBUS_ADDRESS_ADDR EEPROM_CHECK_ADDR+1 //1
#define EEPROM_MODE EEPROM_MODBUS_ADDRESS_ADDR+1 //1
#define EEPROM_MAX_POS EEPROM_MODE+1 //1
#define EEPROM_MIN_POS EEPROM_MAX_POS+1 //1
#define EEPROM_MAX_VEL EEPROM_MIN_POS+1 //1
#define EEPROM_MAX_CURR EEPROM_MAX_VEL+1 //1
#define EEPROM_MIN_VOLTAGE EEPROM_MAX_CURR+1 //1
uint16_t VirtAddVarTab[EEPROM_NB_OF_VAR]={EEPROM_CHECK_ADDR, EEPROM_MODBUS_ADDRESS_ADDR, EEPROM_MODE, EEPROM_MAX_POS, EEPROM_MIN_POS, EEPROM_MAX_VEL, EEPROM_MAX_CURR, EEPROM_MIN_VOLTAGE};
void EEPROM_Init(void);

//########################################
int main(void)
{
	uint16_t debug_cnt=20;

	System_Init(); //inicjalizacja podstawowa: zegar, systick, watchdogi
    DEBUG_Init(); //inicjalizacja uartu do debuggowania
    LEDs_Init(); //init LED
    Switch_Init(); //init switch
    IO_Init(); //init IOs
    MODBUS_Init(); //wlaczenie obslugi MODBUSa
//    ENCODER_Init();
//    AMS_Init(); //wlaczenie obslugi AMSa
//    MOTOR_Init();
    EEPROM_Init(); //inicjacja pamieci i przywrocenie zmiennych


    while (1)
    {
        MODBUS_Task();
        ModbusTableTask();
        if(sys_state.status&STATUS_STARTED)
		{
//        	config.calc_val = Calc(config.setpoint,ENCODER_VAL);
//        	MOTOR_Move( config.calc_val );
		}

        if(tick_1ms)
        {
            tick_1ms=0;
            MODBUS_Timer();
        }
        if(tick_10ms)
        {
            tick_10ms=0;
        }
        if(tick_100ms)
        {
            tick_100ms=0;
            LedTask();
            SwitchTask();

            debug_cnt--;
            if(debug_cnt==0)
            {
            	debug_cnt = 20;
            	DEBUG_print_text("jeszcze zyje\n");
            }
        }
    }
}

//########################################
void ModbusTableTask(void)
{
    bool semafor=false;

    if(modbus_tasks.flag) //jesli przyszlo polecenie z RSa
    {
        if( (modbus_tasks.table[modbus_tasks.tail].adres & 0xF000) == 0x3000 )
        {
            semafor = ModbusTable_3_space();
        }
        else if( (modbus_tasks.table[modbus_tasks.tail].adres & 0xF000) == 0x2000 )
        {
            semafor = ModbusTable_2_space();
        }
        else if( (modbus_tasks.table[modbus_tasks.tail].adres & 0xF000) == 0x1000 )
        {
            semafor = ModbusTable_1_space();
        }
        else if( (modbus_tasks.table[modbus_tasks.tail].adres & 0xF000) == 0x0000 )
        {
            semafor = ModbusTable_0_space();
        }
        else {semafor = true;}


        if(semafor)
        {
            modbus_tasks.tail++; //zwieksz ogon
            if(modbus_tasks.tail>=MODBUS_SYSTEM_BUFOR_SIZE) modbus_tasks.tail = 0; //jesli doszedl do konca bufora to zapetl
            if(modbus_tasks.tail==modbus_tasks.head) modbus_tasks.flag = 0; //jesli osiagnieto glowe to wystaw flage zakonczenia zadania
        }
    }
}
void LedTask(void)
{
    static uint16_t counter=19;

    if(counter && !(counter%19) && modbus_statistics.tx_counter) {modbus_statistics.tx_counter = 0; LED_USR(1);}
    else if(counter && !(counter%17) && modbus_statistics.rx_counter) {modbus_statistics.rx_counter = 0; LED_USR(1);}
    else if(counter && !(counter%15) && modbus_settings.address!=0) {LED_USR(1);}
    else LED_USR(0);

    if(counter) counter--;
    else counter=19;
}
void SwitchTask(void)
{
	if(!SWITCH_CH1) modbus_settings.mute = true;
	else modbus_settings.mute = false;
	if(SWITCH_CH2) debug_mute = true;
	else debug_mute = false;
}

//########################################
void System_Init(void)
{
//FLASH
    FLASH->ACR = FLASH_ACR_PRFTBE|FLASH_ACR_LATENCY_2; //cache i opoznienie w dostepie do pamieci

//RCC PLL 72MHz
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    RCC->CR |= RCC_CR_HSEON;
    while((RCC->CR & RCC_CR_HSERDY) == 0){} //oczekiwanie na HSE
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV|RCC_CFGR_PLLMUL_0|RCC_CFGR_PLLMUL_1|RCC_CFGR_PLLMUL_2; //HSE jako zrodlo dla PLL, PLL_MUL=9 (72MHz)
    RCC->CR |= RCC_CR_PLLON; //wlaczenie PLL
    while((RCC->CR & RCC_CR_PLLRDY) == 0){} //oczekiwanie na PLL
    RCC->CFGR |= RCC_CFGR_SW_PLL; //ustawienie PLL jako zegar systemowy
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL){} //oczekiwanie na ustawienie PLL
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; //APB1 (36MHz)
    RCC->CFGR3 |= RCC_CFGR3_TIM1SW_PLL; //TIM1 (144MHz)

//FPU
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2)); //dostep

//SYSTICK
    SysTick->LOAD = 72000; //tick co 1ms
    SysTick->VAL = 0; //Zerowanie timera
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk|SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_CLKSOURCE_Msk; //Wl. SysTick

//IWDG watchdog= 0.4095s
    //    IWDG->KR = 0xCCCC; //aktywacja watchdoga
    //    IWDG->KR = 0xAAAA; //odswiezenie licznika

//WWDG= 43.69ms
    //    RCC->APB1ENR |= RCC_APB1ENR_WWDGEN; //wlaczenie zasilania watchdoga
    //    WWDG->CFR = 1<<9|0x3<<7|0x7F; //wl. przerwania, preskaler 8, window max
    //    NVIC->ISER[0] |= 1<<0; //wl. przerwania w NVIC
    //    WWDG->CR = WWDG_CR_WDGA|0x7F; //wlaczenie watchdoga
}
void ADC_Init(void)
{
	//ADC2-IN1 PA4 --voltage
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; //Wl. zasilania dla wejscia analogowego
	GPIOA->MODER |= GPIO_MODER_MODER4; //Wyprowadzenie jako wejscie analogowe

	RCC->AHBENR |= RCC_AHBENR_ADC12EN; //Wl. zasilan ia dla ADC



}
void LEDs_Init(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN; //Wl. zasilania dla diody LED
	GPIOB->MODER |= GPIO_MODER_MODER14_0; //Wyprowadzenie jako wyjscie
	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR14; //High speed
}
void Switch_Init(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN; //Wl. zasilania dla switcha
	GPIOC->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR13|GPIO_OSPEEDER_OSPEEDR14; //High speed
}
void IO_Init(void)
{
	//wejscia PB15, PC15
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN; //Wl. zasilania dla wejscia
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN; //Wl. zasilania dla wejscia
	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR15; //High speed
	GPIOC->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR15; //High speed

	//wyjscia PB9(TIM4CH4), PB8(TIM4CH3)
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN; //Wl. zasilania
    GPIOB->AFR[1] |= 0x2<<4 | 0x2<<0; //Funkcje alternatywne dla pinow
    GPIOB->MODER |= GPIO_MODER_MODER9_1|GPIO_MODER_MODER8_1; //Funkcje alternatywne
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR9|GPIO_OSPEEDER_OSPEEDR8; //High speed

    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; //Wl. zasilania licznika 4
    TIM4->CR1 |= TIM_CR1_ARPE; //buforowanie ARR
    TIM4->CCMR2 |= TIM_CCMR2_OC3M|TIM_CCMR2_OC4M|TIM_CCMR2_OC3PE|TIM_CCMR2_OC4PE; //tryb PWM
    TIM4->CCER |= TIM_CCER_CC3E|TIM_CCER_CC4E|TIM_CCER_CC3P|TIM_CCER_CC4P; //wl wyjscia PWM z licznika
    TIM4->ARR = 0xFF; //wartosc graniczna pwm
    TIM4->EGR |= TIM_EGR_UG; //aktualizuj rejestry
    TIM4->CR1 |= TIM_CR1_CEN; //wl licznika
}
void EEPROM_Init(void)
{
    uint16_t data;

    FLASH_Unlock();
    EE_Init();

    EE_ReadVariable(EEPROM_CHECK_ADDR,&data);
    if(data==EEPROM_VALID_MARKER)
    {
        EE_ReadVariable(EEPROM_MODBUS_ADDRESS_ADDR,&data);
        modbus_settings.address = data;
        EE_ReadVariable(EEPROM_MODE,&data);
        sys_state.mode = (uint8_t)data;
        EE_ReadVariable(EEPROM_MAX_POS,&data);
        sys_state.max_pos = data;
        EE_ReadVariable(EEPROM_MIN_POS,&data);
        sys_state.min_pos = data;
        EE_ReadVariable(EEPROM_MAX_VEL,&data);
        sys_state.max_vel = data;
        EE_ReadVariable(EEPROM_MAX_CURR,&data);
        sys_state.max_curr = data;
        EE_ReadVariable(EEPROM_MIN_VOLTAGE,&data);
        sys_state.min_volatge = data;
    }
    else
    {
        data = sys_state.min_volatge;
        EE_WriteVariable(EEPROM_MIN_VOLTAGE,data);
        data = sys_state.max_curr;
        EE_WriteVariable(EEPROM_MAX_CURR,data);
        data = sys_state.max_vel;
        EE_WriteVariable(EEPROM_MAX_VEL,data);
        data = sys_state.min_pos;
        EE_WriteVariable(EEPROM_MIN_POS,data);
        data = sys_state.max_pos;
        EE_WriteVariable(EEPROM_MAX_POS,data);
        data = (uint16_t)sys_state.mode;
        EE_WriteVariable(EEPROM_MODE,data);
        data = MODBUS_ADDR;
        EE_WriteVariable(EEPROM_MODBUS_ADDRESS_ADDR,data);
        data = EEPROM_VALID_MARKER;
        EE_WriteVariable(EEPROM_CHECK_ADDR,data);
    }
}
