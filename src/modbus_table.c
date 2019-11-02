#include "modbus_table.h"

bool ModbusTable_0_space(void)
{
//	float tmp;
//
//    if(modbus_tasks.table[modbus_tasks.tail].adres == 0x0000) //czas
//    {
//        if(modbus_tasks.table[modbus_tasks.tail].status != TASK_ODCZYT)
//        {
//            config.time = modbus_tasks.table[modbus_tasks.tail].wartosc;
//            if(config.time<1) config.time=1;
//        }
//    }
//    else if(modbus_tasks.table[modbus_tasks.tail].adres == modbus_settings.address) //zadana
//    {
//        if(modbus_tasks.table[modbus_tasks.tail].status != TASK_ODCZYT)
//        {
//            config.setpoint = modbus_tasks.table[modbus_tasks.tail].wartosc<<2;

//            config.setvel = ((config.setpoint>>2)-(ENCODER_VAL>>2));
//            config.setvel /= config.time;
//            if(config.setvel<0) config.setvel=-config.setvel;
//            if(config.setvel>1000) config.setvel=1000;
//        }
//    }

    return true;
}
bool ModbusTable_1_space(void)
{
    bool semafor = false;
//    float a,b;
    uint16_t tmp=0;
//
    switch(modbus_tasks.table[modbus_tasks.tail].adres & 0xFFF)
    {
    //########################################
    case 0x000: //status
        if(modbus_tasks.table[modbus_tasks.tail].status == TASK_ODCZYT)
        {
        	tmp |= (debug_mute?1:0) << STATUS_DEBUG;
        	tmp |= (modbus_settings.mute?1:0) << STATUS_MUTE;
        	tmp |= (INPUTS_CH1?1:0) << STATUS_IN1;
        	tmp |= (INPUTS_CH2?1:0) << STATUS_IN2;
            modbus_tasks.table[modbus_tasks.tail].wartosc = tmp;
        }
        else
        {
        }
        semafor = true;
        break;
    //########################################
    case 0x001: //outputs
        if(modbus_tasks.table[modbus_tasks.tail].status == TASK_ODCZYT)
        {
        }
        else
        {
        	DEBUG_print_var("output1",(int16_t)modbus_tasks.table[modbus_tasks.tail].wartosc>>8);
        	DEBUG_print_var("output2",(int16_t)modbus_tasks.table[modbus_tasks.tail].wartosc&0xFF);
        	OUTPUTS_CH1(modbus_tasks.table[modbus_tasks.tail].wartosc>>8);
        	OUTPUTS_CH2(modbus_tasks.table[modbus_tasks.tail].wartosc&0xFF);
        }
        semafor = true;
        break;
    }
    return semafor;
}
bool ModbusTable_2_space(void)
{
    bool semafor = true;
//    uint16_t tmp;
//
//    switch(modbus_tasks.table[modbus_tasks.tail].adres & 0xFFF)
//    {
//    //########################################
//    case 0x000: //adres modbusa
//        if(modbus_tasks.table[modbus_tasks.tail].status == TASK_ODCZYT)
//        {
//            modbus_tasks.table[modbus_tasks.tail].wartosc=modbus_settings.address;
//        }
//        else
//        {
//            if(modbus_tasks.table[modbus_tasks.tail].wartosc&0x8000)
//            {
//            	modbus_settings.address = (uint8_t)modbus_tasks.table[modbus_tasks.tail].wartosc&0xFFF;
////                EE_WriteVariable(EEPROM_MODBUS_ADDRESS_ADDR,(uint16_t)modNrSterownika);
//            }
//        }
//        semafor = true;
//        break;
//    //########################################
//    }
//    return semafor;
//}
//bool ModbusTable_3_space(void)
//{
//    bool semafor = false;
//    uint16_t tmp;

    return semafor;
}
bool ModbusTable_3_space(void)
{
    bool semafor = true;
//    uint16_t tmp;
//
//    switch(modbus_tasks.table[modbus_tasks.tail].adres & 0xFFF)
//    {
//    //########################################
//    case 0x000: //adres modbusa
//        if(modbus_tasks.table[modbus_tasks.tail].status == TASK_ODCZYT)
//        {
//            modbus_tasks.table[modbus_tasks.tail].wartosc=modbus_settings.address;
//        }
//        else
//        {
//            if(modbus_tasks.table[modbus_tasks.tail].wartosc&0x8000)
//            {
//            	modbus_settings.address = (uint8_t)modbus_tasks.table[modbus_tasks.tail].wartosc&0xFFF;
////                EE_WriteVariable(EEPROM_MODBUS_ADDRESS_ADDR,(uint16_t)modNrSterownika);
//            }
//        }
//        semafor = true;
//        break;
//    //########################################
//    }
//    return semafor;
//}
//bool ModbusTable_3_space(void)
//{
//    bool semafor = false;
//    uint16_t tmp;

    return semafor;
}
