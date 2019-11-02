#include "modbus.h"

//Obslugiwane funkcje
#define MOD_ODCZ_N 0x03 //odczyt n rejestrow
#define MOD_ZAP_1 0x06 //zapis 1 rejestru
#define MOD_ZAP_N 0x10 //zapis n rejestrow
#define MOD_ID 0x11 //identyfikacja

//######################################## RXD
volatile struct modRxd_t{
	uint8_t adres; //odebrany adres modulu
	uint8_t funkcja; //odebrana funkcja
	uint16_t rejestr; //odebrany adres rejestru
	uint16_t liczba_rej; //odebrana liczba rejestrow
	uint8_t dane[MOD_ILOSC_REJ*2]; //bufor danych
	uint16_t crc; //odebrane crc
	uint8_t blad;
	bool settling;
}modRxd;

//######################################## TXD
#define PUT2TXD_Start 0 //Pierwszy bajt
#define PUT2TXD_Cont 1 //Kolejny bajt, bedzie kontynuacja
#define PUT2TXD_Last 2 //Start wysylania

volatile struct modTxd_t{
	uint8_t licz; //licznik do wysylania z bufora TxD
	uint8_t ind; //wskaznik do wysylania z bufora TxD
	uint8_t dane[3*2+MOD_ILOSC_REJ*4+2+1]; //dlugosc paczki (+1)
	bool settling;
}modTxd;

//######################################## KONTROLA PRZEPLYWU
volatile enum modbus_status{
	MODBUS_IDLE=0,
	MODBUS_ODEBRANO,
	MODBUS_OCZEKIWANIE,
	MODBUS_ODPOWIEDZ,
	MODBUS_ODP_WAIT,
}modbusStatus = MODBUS_IDLE; //faza przetwarzania rozkazu
uint16_t tim_noanswer; //timer odliczajacy czas oczekiwania na uzyskanie odpowiedzi

//######################################## WYMIANA DANYCH Z RESZTA SYSTEMU i USTAWIENIA
modbus_tasks_t modbus_tasks;
volatile modbus_statistics_t modbus_statistics;
modbus_settings_t modbus_settings={MODBUS_ADDR,MODBUS_115200};

//######################################## PROTOTYPY
static void MODBUS_put2txd(unsigned char c, unsigned char etap);
static bool MODBUS_AddTaskForSystem(uint16_t adres,uint16_t wartosc,enum modbus_taskStatus status);
static uint16_t MODBUS_Rx_Crc(uint8_t reset,uint8_t new);
static void MODBUS_RTU(uint8_t c);

//########################################
bool MODBUS_AddTaskForSystem(uint16_t adres,uint16_t wartosc,enum modbus_taskStatus status)
{
	uint16_t tmp;

	modbus_tasks.table[modbus_tasks.head].adres = adres;
	modbus_tasks.table[modbus_tasks.head].wartosc = wartosc;
	modbus_tasks.table[modbus_tasks.head].status = status;
	modbus_tasks.flag = 1;

	tmp = modbus_tasks.head + 1;
	if(tmp >= MODBUS_SYSTEM_BUFOR_SIZE) tmp = 0;
	if(tmp == modbus_tasks.tail)
	{
		modbus_statistics.system_buff_overload ++;
		return false;
	}
	else
	{
		modbus_tasks.head = tmp;
		return true;
	}
}

//######################################## Zerowanie timerow (musi byc wykonywane cyklicznie co 1ms)
void MODBUS_Timer(void)
{
	if(tim_noanswer) tim_noanswer--;
}

//######################################## Task
void MODBUS_Task(void)
{
	uint16_t k;

	switch(modbusStatus)
	{
	//######################################## ODBIOR
	case MODBUS_ODEBRANO:
		DEBUG_print_text("MODBUS odebralem\n");
		modbus_statistics.rx_counter++;
		if(modRxd.funkcja==MOD_ZAP_1 || modRxd.funkcja==MOD_ZAP_N) //funkcje dla adresu ogolnego jak i indywidualnego (ZAPIS)
		{
			modbus_tasks.head_shadow = modbus_tasks.head; //zapisanie stanu glowy na potem
			for(k=modRxd.rejestr;k<(modRxd.rejestr+(modRxd.liczba_rej>>1));k++) //dla wszystkich rejestrow
			{
				if(!MODBUS_AddTaskForSystem(k,modRxd.dane[(k-modRxd.rejestr)*2]<<8 | modRxd.dane[(k-modRxd.rejestr)*2+1],TASK_ZAPIS))//dodanie rejestru do zadania
				{
					modRxd.blad = 6; //jesli bufor systemowy zapelniony to daj komunikat o zajetosci
					break;
				}
			}
			modbusStatus = MODBUS_ODPOWIEDZ; //odpowiedz juz teraz
		}
		else if(modbus_settings.address && modRxd.adres==modbus_settings.address) //funkcje tylko dla adresu indywidualnego (ODCZYT)
		{
			if(modRxd.funkcja==MOD_ODCZ_N) //dla odczytu rejestrow
			{
				modbus_tasks.head_shadow = modbus_tasks.head; //zapisanie stanu glowy na potem
				for(k=modRxd.rejestr;k<(modRxd.rejestr+(modRxd.liczba_rej>>1));k++) //dla wszystkich rejestrow
				{
					if(!MODBUS_AddTaskForSystem(k,0,TASK_ODCZYT))//dodanie rejestru do rzadania
					{
						modRxd.blad = 6; //jesli bufor systemowy zapelniony to daj komunikat o zajetosci
						break;
					}
				}
				tim_noanswer = MODBUS_MAX_TIME; //ustal czas oczekiwania na odpowiedz
				modbusStatus = MODBUS_OCZEKIWANIE; //przejdz do oczekiwania
			}
			else if(modRxd.funkcja==MOD_ID) modbusStatus = MODBUS_ODPOWIEDZ; //dla komunikatu identyfikacyjnego
		}
		else
		{
			modRxd.blad = 1; //nieobslugiwana funkcja
			modbusStatus = MODBUS_ODPOWIEDZ; //odpowiedz juz teraz
		}
		break;
	//######################################## OCZEKIWANIE
	case MODBUS_OCZEKIWANIE:
		if(!tim_noanswer || !modbus_tasks.flag) //jesli system zgromadzil dane lub czas minal
		{
			if(modbus_tasks.flag)
			{
				modRxd.blad = 6; //przekroczono czas wykonania rozkazu- nadaj komunikat o zajetosci
				modbus_tasks.head_shadow = modbus_tasks.head; //nie odpowiadaj juz na ta wiadomosc
			}
			modbusStatus = MODBUS_ODPOWIEDZ;
			tim_noanswer = 0;
		}
		break;
	//######################################## REDAGOWANIE ODPOWIEDZI
	case MODBUS_ODPOWIEDZ:
		DEBUG_print_text("MODBUS odpowiadam\n");
		if(modbus_settings.address && modRxd.adres==modbus_settings.address) //jesli urzaczenie wywolano po indywidualnym adresie
		{
			if(modRxd.blad) //jesli kod bledu- sprawa prosta
			{
				MODBUS_put2txd(modRxd.adres,PUT2TXD_Start);
				MODBUS_put2txd(modRxd.funkcja|0x80,PUT2TXD_Cont);
				MODBUS_put2txd(modRxd.blad,PUT2TXD_Cont);
				MODBUS_put2txd(0,PUT2TXD_Last);
				modRxd.blad = 0;
			}
			else //jesli zwykla odpowiedz
			{
				switch(modRxd.funkcja)
				{
				case MOD_ODCZ_N: //odczyt wielu
					MODBUS_put2txd(modRxd.adres,PUT2TXD_Start);
					MODBUS_put2txd(MOD_ODCZ_N,PUT2TXD_Cont);
					MODBUS_put2txd(modRxd.liczba_rej&0xFF,PUT2TXD_Cont); //wstep do wiadomosci
					do{
						if(modbus_tasks.table[modbus_tasks.head_shadow].status==TASK_ODCZYT) //spr. czy wszystkie komorki wczytano
						{
							MODBUS_put2txd((uint8_t)((modbus_tasks.table[modbus_tasks.head_shadow].wartosc>>8)&0xFF),PUT2TXD_Cont); //laduje rejestry
							MODBUS_put2txd((uint8_t)(modbus_tasks.table[modbus_tasks.head_shadow].wartosc&0xFF),PUT2TXD_Cont); //do wiadomosci
						}
						else
						{
							modRxd.blad = 2; // jesli nie wczytano jakiegos rejestru to blad
							return;
						}
						modbus_tasks.head_shadow++;
						if(modbus_tasks.head_shadow>=MODBUS_SYSTEM_BUFOR_SIZE) modbus_tasks.head_shadow = 0;
					}while(modbus_tasks.head_shadow!=modbus_tasks.head);

					MODBUS_put2txd(0,PUT2TXD_Last); //jesli wszystkie wczytano to pusc dalej
					break;
				case MOD_ZAP_1: //zapis jednego
					if(modbus_tasks.table[modbus_tasks.head_shadow].status!=TASK_ZAPIS) // spr. czy zapisano komorke
					{
						modRxd.blad = 2; //jesli nie zapisano to wyrzuc blad
						return;
					}
					MODBUS_put2txd(modRxd.adres,PUT2TXD_Start); //jesli jednak wszystko poszlo OK to nadaj potwierdzenie
					MODBUS_put2txd(MOD_ZAP_1,PUT2TXD_Cont);
					MODBUS_put2txd((modRxd.rejestr&0xFF00)>>8,PUT2TXD_Cont);
					MODBUS_put2txd(modRxd.rejestr&0xFF,PUT2TXD_Cont);
					MODBUS_put2txd(modRxd.dane[0],PUT2TXD_Cont);
					MODBUS_put2txd(modRxd.dane[1],PUT2TXD_Cont);
					MODBUS_put2txd(0,PUT2TXD_Last);
					break;
				case MOD_ZAP_N: //zapis wielu
					do{
						if(modbus_tasks.table[modbus_tasks.head_shadow].status!=TASK_ZAPIS) //sprawdzenie wykonania zadan zapisu
						{
							modRxd.blad = 2; //jesli cos poszlo zle to komunikat o bledzie
							return;
						}
						modbus_tasks.head_shadow++;
						if(modbus_tasks.head_shadow>=MODBUS_SYSTEM_BUFOR_SIZE) modbus_tasks.head_shadow = 0;
					}while(modbus_tasks.head_shadow!=modbus_tasks.head);

					MODBUS_put2txd(modRxd.adres,PUT2TXD_Start); //jesli wszystko poszlo OK to nadaj potwierdzenie
					MODBUS_put2txd(MOD_ZAP_N,PUT2TXD_Cont);
					MODBUS_put2txd((modRxd.rejestr&0xFF00)>>8,PUT2TXD_Cont);
					MODBUS_put2txd(modRxd.rejestr&0xFF,PUT2TXD_Cont);
					MODBUS_put2txd(((modRxd.liczba_rej>>1)&0xFF00)>>8,PUT2TXD_Cont);
					MODBUS_put2txd((modRxd.liczba_rej>>1)&0xFF,PUT2TXD_Cont);
					MODBUS_put2txd(0,PUT2TXD_Last);
					break;
				case MOD_ID: //wyslij ID
					MODBUS_put2txd(modRxd.adres,PUT2TXD_Start);
					MODBUS_put2txd(MOD_ID,PUT2TXD_Cont);
					MODBUS_put2txd(3,PUT2TXD_Cont);
					MODBUS_put2txd((uint8_t)(MOD_DEV_TYPE>>8),PUT2TXD_Cont);
					MODBUS_put2txd((uint8_t)(MOD_DEV_TYPE&0xFF),PUT2TXD_Cont);
					MODBUS_put2txd(0xFF,PUT2TXD_Cont);
					MODBUS_put2txd(0,PUT2TXD_Last);
				}
			}
			modbusStatus = MODBUS_ODP_WAIT; //oczekuj na mozliwosc wyslania zbuforowanej wiadomosci
		}
		else modbusStatus = MODBUS_IDLE;
		break;
	//######################################## INICJACJA WYSYLANIA ODPOWIEDZI
	case MODBUS_ODP_WAIT:
		if(!modTxd.settling) //jesli timer sie wyzerowal i mamy wiadomosc do nadania to rozpocznij
		{
			modbus_statistics.tx_counter++;
			modTxd.licz--;
			USART_TXD_REGISTER = modTxd.dane[modTxd.ind++]; //nadaj pierwszy bajt
			modbusStatus = MODBUS_IDLE; //przejdz do stanu oczekiwania
		}
		break;
	//######################################## NIC SIE NIE DZIEJE
	case MODBUS_IDLE:
		break;
	}
}

//######################################## wypychanie danych przez modbus
void MODBUS_put2txd(unsigned char c, unsigned char etap)
{
	switch(etap)
	{
		case 0: //poczatek
			modTxd.ind=0;
			CRC_RESET;
		case 1: //srodek
			modTxd.dane[modTxd.ind++]=c;
			CRC_CALC(c);
			break;
		case 2: //koniec-wyslij
			modTxd.dane[modTxd.ind++]=CRC_GET_LOW;
			modTxd.dane[modTxd.ind++]=CRC_GET_HIGH;
			modTxd.licz=modTxd.ind;
			modTxd.ind=0;
	}
}

//########################################
void MODBUS_Handler(void)
{
    uint8_t c;
    //########################################
    if(USART_RXD_ISR_COMPLETE) //odbiornik
    {
        c = USART_RXD_REGISTER; //odczyt znaku-likwidacja flagi
        if(modTxd.licz) return; //likwidacja echa-odbierania wysylanych przez siebie znakow
        if(modbus_settings.mute==false) MODBUS_RTU(c); //rozpatruj przychodzace znaki jako modbus RTU
    }
    //########################################
    if(USART_SETTLING_ISR_COMPLETE) //settling timer
    {
    	USART_SETTLING_ISR_CLEAR; //czyszczenie flagi pomiaru settling time
    	modRxd.settling = false; //zezwalaj na odbior nowych wiadomosci
    	modTxd.settling = false; //zezwalaj na wysylanie nowych wiadomosci
    }
    //########################################
    else //nadajnik
    {
        USART_TXD_ISR_CLEAR; //czyszczenie flagi zapisu
        if(modTxd.licz) //jesli jest cos jeszcze do zapisu
        {
            modTxd.licz--;
            USART_TXD_REGISTER = modTxd.dane[modTxd.ind++]; //wypchnij kolejny znak
        }
        else
        {
        	modTxd.settling = true; //czekamy na settling
        }
    }
    //########################################
}

//########################################
static uint16_t MODBUS_Rx_Crc(uint8_t reset,uint8_t new)
{
	static uint8_t data[MOD_ILOSC_REJ*2+6];
	static uint8_t* data_ind=data;
	uint8_t* tmp_ind=data;

	if(reset==0)
	{
		CRC_RESET;
		data_ind = data;
		*data_ind=new;
		data_ind++;
	}
	else if(reset==1)
	{
		*data_ind=new;
		data_ind++;
	}
	else
	{
		while(tmp_ind!=data_ind) CRC_CALC(*(tmp_ind++));
		data_ind = data;
		return (CRC_GET_HIGH<<8|CRC_GET_LOW);
	}

	return 0;
}
static void MODBUS_RTU(uint8_t c)
{
	static uint8_t modRInd; //wskaznik bufora danych
	static enum {
		MODBUS_ADRES,
		MODBUS_FUNKCJA,
		MODBUS_REJESTR_HI,
		MODBUS_REJESTR_LO,
		MODBUS_LICZBA_REJ_HI,
		MODBUS_LICZBA_REJ_LO,
		MODBUS_LICZBA_BAJ,
		MODBUS_DANE_HI,
		MODBUS_DANE_LO,
		MODBUS_CRC_LO,
		MODBUS_CRC_HI,
	}modbusFazaOdbioru = MODBUS_ADRES; //faza odbieranej ramki

	if(modbusFazaOdbioru==MODBUS_ADRES && modRxd.settling)
	{
		return; //jesli znak przyszedl zbyt wczesnie to nie akceptuj
	}
	//-------------------------------------------------------
	switch(modbusFazaOdbioru)
	{
	case MODBUS_ADRES:
		if(c==modbus_settings.address || c==0)
		{
			modRxd.adres = c;
			MODBUS_Rx_Crc(0,c);
			modbusFazaOdbioru = MODBUS_FUNKCJA;
		}
		modRxd.settling = true;
		break;
	//-------------------------------------------------------
	case MODBUS_FUNKCJA:
		modRxd.funkcja = c;
		if(c==MOD_ODCZ_N || c==MOD_ZAP_1 || c==MOD_ZAP_N || c==MOD_ID)
		{
			MODBUS_Rx_Crc(1,c);
			if(c==MOD_ID) modbusFazaOdbioru = MODBUS_CRC_LO;
			else modbusFazaOdbioru = MODBUS_REJESTR_HI;
		}
		else
		{
			modbusFazaOdbioru = MODBUS_ADRES;
			modRxd.blad = 1;// niedozwolona funkcja
			modbusStatus = MODBUS_ODPOWIEDZ;
		}
		break;
	//-------------------------------------------------------
	case MODBUS_REJESTR_HI:
		modRxd.rejestr = c<<8;
		MODBUS_Rx_Crc(1,c);
		modbusFazaOdbioru = MODBUS_REJESTR_LO;
		break;
	//-------------------------------------------------------
	case MODBUS_REJESTR_LO:
		modRxd.rejestr |= c;
		MODBUS_Rx_Crc(1,c);
		if(modRxd.funkcja==MOD_ZAP_1)
		{
			modRxd.liczba_rej = 2;
			modRInd = 0;
			modbusFazaOdbioru = MODBUS_DANE_HI;
		}
		else modbusFazaOdbioru = MODBUS_LICZBA_REJ_HI;
		break;
	//-------------------------------------------------------
	case MODBUS_LICZBA_REJ_HI:
		modRxd.liczba_rej = c<<8;
		MODBUS_Rx_Crc(1,c);
		modbusFazaOdbioru = MODBUS_LICZBA_REJ_LO;
		break;
	//-------------------------------------------------------
	case MODBUS_LICZBA_REJ_LO:
		modRxd.liczba_rej |= c;
		modRxd.liczba_rej *= 2;
		MODBUS_Rx_Crc(1,c);
		if(modRxd.funkcja== MOD_ZAP_N) modbusFazaOdbioru = MODBUS_LICZBA_BAJ;
		else modbusFazaOdbioru = MODBUS_CRC_LO;
		break;
	//-------------------------------------------------------
	case MODBUS_LICZBA_BAJ:
		if(c==modRxd.liczba_rej)
		{
			MODBUS_Rx_Crc(1,c);
			modRInd = 0;
			modbusFazaOdbioru = MODBUS_DANE_HI;
		}
		else modbusFazaOdbioru = MODBUS_ADRES;
		break;
	//-------------------------------------------------------
	case MODBUS_DANE_HI:
		if(modRInd<(MOD_ILOSC_REJ*2))
		{
			modRxd.dane[modRInd++] = c;
			MODBUS_Rx_Crc(1,c);
			if(modRInd>modRxd.liczba_rej) modbusFazaOdbioru = MODBUS_CRC_LO;
			else modbusFazaOdbioru = MODBUS_DANE_LO;
		}
		else modbusFazaOdbioru = MODBUS_ADRES;
		break;
	//-------------------------------------------------------
	case MODBUS_DANE_LO:
		if(modRInd<(MOD_ILOSC_REJ*2))
		{
			modRxd.dane[modRInd++] = c;
			MODBUS_Rx_Crc(1,c);
			if(modRInd>=modRxd.liczba_rej) modbusFazaOdbioru = MODBUS_CRC_LO;
			else modbusFazaOdbioru = MODBUS_DANE_HI;
		}
		else modbusFazaOdbioru = MODBUS_ADRES;
		break;
	//-------------------------------------------------------
	case MODBUS_CRC_LO:
		modRxd.crc = c;
		modbusFazaOdbioru = MODBUS_CRC_HI;
		break;
	//-------------------------------------------------------
	case MODBUS_CRC_HI:
		modRxd.crc |= c<<8;
		if(modRxd.crc == MODBUS_Rx_Crc(2,0)) modbusStatus = MODBUS_ODEBRANO; //rozpoczyna przetwarzanie odebranych danych- dane poprawne
		modbusFazaOdbioru = MODBUS_ADRES;
		break;
	//-------------------------------------------------------
	default:
		modbusFazaOdbioru = MODBUS_ADRES;
	}
}
