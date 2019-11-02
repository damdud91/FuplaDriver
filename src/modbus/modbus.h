#ifndef MODBUS_H_
#define MODBUS_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f302xc.h"
#include "modbus_config.h"
#include "main.h"

//########################################
#define MOD_DEV_TYPE 0xDD //typ urzadzenia
#define MODBUS_MAX_TIME 2000 //maksymalny czas opoznienia podczas odpowiedzi
#define MOD_ILOSC_REJ 20 //maksymalna ilosc rejestrow odczytywanych za jednym razem
#define MODBUS_SYSTEM_BUFOR_SIZE 128 //wielkosc bufora danych systemowych

enum modbus_taskStatus{
	TASK_ZAPIS,
	TASK_ODCZYT,
	TASK_ZABRONIONE,
};

typedef struct{
	struct taskTable_t{
		uint16_t adres;
		uint16_t wartosc;
		enum modbus_taskStatus status;
	} table[MODBUS_SYSTEM_BUFOR_SIZE];

	uint16_t head;
	uint16_t head_shadow;
	uint16_t tail;
	uint8_t flag;
}modbus_tasks_t;

typedef struct{
	uint16_t system_buff_overload;
	uint16_t rx_buff_overload;
	uint16_t tx_buff_overload;
	uint16_t rx_counter;
	uint16_t tx_counter;
}modbus_statistics_t;

typedef struct{
	uint16_t address;
	uint16_t baudrate;
	bool mute;
}modbus_settings_t;

extern modbus_tasks_t modbus_tasks;
extern volatile modbus_statistics_t modbus_statistics;
extern modbus_settings_t modbus_settings;

//########################################
void MODBUS_Timer(void);
void MODBUS_Task(void);
void MODBUS_Handler(void);

#endif /* MODBUS_H_ */
