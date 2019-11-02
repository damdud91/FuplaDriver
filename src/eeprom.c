/* Includes ------------------------------------------------------------------*/
#include "eeprom.h"

/* Global variable used to store variable value in read sequence */
uint16_t DataVar = 0;

/* Virtual address defined by the user: 0xFFFF value is prohibited */
extern uint16_t VirtAddVarTab[NB_OF_VAR];

static FLASH_Status EE_Format(void);
static uint16_t EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data);
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint16_t Data);
static uint16_t EE_FindValidPage(uint8_t Operation);

uint16_t EE_Init(void)
{
	uint16_t PageStatus0 = 6, PageStatus1 = 6;
	uint16_t VarIdx = 0;
	uint16_t EepromStatus = 0, ReadStatus = 0;
	int16_t x = -1;
	uint16_t  FlashStatus;

	PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS);
	PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS);

	/* Check for invalid header states and repair if necessary */
	switch (PageStatus0)
	{
	case ERASED:
		if (PageStatus1 == VALID_PAGE) /* Page0 erased, Page1 valid */
		{
			FlashStatus = FLASH_ErasePage(PAGE0_BASE_ADDRESS); /* Erase Page0 */
			if (FlashStatus != FLASH_COMPLETE) /* If erase operation was failed, a Flash error code is returned */
			{
				return FlashStatus;
			}
		}
		else if (PageStatus1 == RECEIVE_DATA) /* Page0 erased, Page1 receive */
		{
			FlashStatus = FLASH_ErasePage(PAGE0_BASE_ADDRESS); /* Erase Page0 */
			if (FlashStatus != FLASH_COMPLETE) /* If erase operation was failed, a Flash error code is returned */
			{
				return FlashStatus;
			}
			FlashStatus = FLASH_ProgramHalfWord(PAGE1_BASE_ADDRESS, VALID_PAGE); /* Mark Page1 as valid */
			if (FlashStatus != FLASH_COMPLETE) /* If program operation was failed, a Flash error code is returned */
			{
				return FlashStatus;
			}
		}
		else /* First EEPROM access (Page0&1 are erased) or invalid state -> format EEPROM */
		{
			FlashStatus = EE_Format(); /* Erase both Page0 and Page1 and set Page0 as valid page */
			if (FlashStatus != FLASH_COMPLETE)  /* If erase/program operation was failed, a Flash error code is returned */
			{
				return FlashStatus;
			}
		}
		break;
		//---
		case RECEIVE_DATA:
			if (PageStatus1 == VALID_PAGE) /* Page0 receive, Page1 valid */
			{
				for (VarIdx = 0; VarIdx < NB_OF_VAR; VarIdx++) /* Transfer data from Page1 to Page0 */
				{
					if (( *(__IO uint16_t*)(PAGE0_BASE_ADDRESS + 6)) == VirtAddVarTab[VarIdx])
					{
						x = VarIdx;
					}
					if (VarIdx != x)
					{
						ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar); /* Read the last variables' updates */
						if (ReadStatus != 0x1) /* In case variable corresponding to the virtual address was found */
						{
							EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar); /* Transfer the variable to the Page0 */
							if (EepromStatus != FLASH_COMPLETE) /* If program operation was failed, a Flash error code is returned */
							{
								return EepromStatus;
							}
						}
					}
				}
				FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, VALID_PAGE); /* Mark Page0 as valid */
				if (FlashStatus != FLASH_COMPLETE) /* If program operation was failed, a Flash error code is returned */
				{
					return FlashStatus;
				}
				FlashStatus = FLASH_ErasePage(PAGE1_BASE_ADDRESS); /* Erase Page1 */
				if (FlashStatus != FLASH_COMPLETE) /* If erase operation was failed, a Flash error code is returned */
				{
					return FlashStatus;
				}
			}
			else if (PageStatus1 == ERASED) /* Page0 receive, Page1 erased */
			{
				FlashStatus = FLASH_ErasePage(PAGE1_BASE_ADDRESS); /* Erase Page1 */
				if (FlashStatus != FLASH_COMPLETE) /* If erase operation was failed, a Flash error code is returned */
				{
					return FlashStatus;
				}
				FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, VALID_PAGE); /* Mark Page0 as valid */
				if (FlashStatus != FLASH_COMPLETE) /* If program operation was failed, a Flash error code is returned */
				{
					return FlashStatus;
				}
			}
			else /* Invalid state -> format eeprom */
			{
				FlashStatus = EE_Format(); /* Erase both Page0 and Page1 and set Page0 as valid page */
				if (FlashStatus != FLASH_COMPLETE) /* If erase/program operation was failed, a Flash error code is returned */
				{
					return FlashStatus;
				}
			}
			break;
			//---
		case VALID_PAGE:
			if (PageStatus1 == VALID_PAGE) /* Invalid state -> format eeprom */
			{
				FlashStatus = EE_Format(); /* Erase both Page0 and Page1 and set Page0 as valid page */
				if (FlashStatus != FLASH_COMPLETE) /* If erase/program operation was failed, a Flash error code is returned */
				{
					return FlashStatus;
				}
			}
			else if (PageStatus1 == ERASED) /* Page0 valid, Page1 erased */
			{
				FlashStatus = FLASH_ErasePage(PAGE1_BASE_ADDRESS); /* Erase Page1 */
				if (FlashStatus != FLASH_COMPLETE) /* If erase operation was failed, a Flash error code is returned */
				{
					return FlashStatus;
				}
			}
			else /* Page0 valid, Page1 receive */
			{
				for (VarIdx = 0; VarIdx < NB_OF_VAR; VarIdx++) /* Transfer data from Page0 to Page1 */
				{
					if ((*(__IO uint16_t*)(PAGE1_BASE_ADDRESS + 6)) == VirtAddVarTab[VarIdx])
					{
						x = VarIdx;
					}
					if (VarIdx != x)
					{
						ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar); /* Read the last variables' updates */
						if (ReadStatus != 0x1)  /* In case variable corresponding to the virtual address was found */
						{
							EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar); /* Transfer the variable to the Page1 */
							if (EepromStatus != FLASH_COMPLETE) /* If program operation was failed, a Flash error code is returned */
							{
								return EepromStatus;
							}
						}
					}
				}
				FlashStatus = FLASH_ProgramHalfWord(PAGE1_BASE_ADDRESS, VALID_PAGE); /* Mark Page1 as valid */
				if (FlashStatus != FLASH_COMPLETE) /* If program operation was failed, a Flash error code is returned */
				{
					return FlashStatus;
				}
				FlashStatus = FLASH_ErasePage(PAGE0_BASE_ADDRESS); /* Erase Page0 */
				if (FlashStatus != FLASH_COMPLETE) /* If erase operation was failed, a Flash error code is returned */
				{
					return FlashStatus;
				}
			}
			break;
			//---
		default:  /* Any other state -> format eeprom */
			FlashStatus = EE_Format();  /* Erase both Page0 and Page1 and set Page0 as valid page */
			if (FlashStatus != FLASH_COMPLETE) /* If erase/program operation was failed, a Flash error code is returned */
			{
				return FlashStatus;
			}
			break;
		}
  return FLASH_COMPLETE;
}
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data)
{
	uint16_t ValidPage = PAGE0;
	uint16_t AddressValue = 0x5555, ReadStatus = 1;
	uint32_t Address = 0x08010000, PageStartAddress = 0x08010000;

	ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE); /* Get active Page for read operation */
	if (ValidPage == NO_VALID_PAGE)  return  NO_VALID_PAGE;/* Check if there is no valid page */
	PageStartAddress = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * PAGE_SIZE)); /* Get the valid Page start Address */
	Address = (uint32_t)((EEPROM_START_ADDRESS - 2) + (uint32_t)((1 + ValidPage) * PAGE_SIZE)); /* Get the valid Page end Address */
	while (Address > (PageStartAddress + 2)) /* Check each active page address starting from end */
	{
		AddressValue = (*(__IO uint16_t*)Address); /* Get the current location content to be compared with virtual address */
		if (AddressValue == VirtAddress) /* Compare the read address with the virtual address */
		{
			*Data = (*(__IO uint16_t*)(Address - 2)); /* Get content of Address-2 which is variable value */
			ReadStatus = 0; /* In case variable value is read, reset ReadStatus flag */
			break;
		}
		else Address = Address - 4; /* Next address location */
	}
	return ReadStatus; /* Return ReadStatus value: (0: variable exist, 1: variable doesn't exist) */
}
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data)
{
	uint16_t Status = 0;
	Status = EE_VerifyPageFullWriteVariable(VirtAddress, Data); /* Write the variable virtual address and value in the EEPROM */
	if (Status == PAGE_FULL) Status = EE_PageTransfer(VirtAddress, Data); /* In case the EEPROM active page is full */ /* Perform Page transfer */
	return Status; /* Return last operation status */
}
static FLASH_Status EE_Format(void)
{
	FLASH_Status FlashStatus = FLASH_COMPLETE;
	FlashStatus = FLASH_ErasePage(PAGE0_BASE_ADDRESS); /* Erase Page0 */
	if (FlashStatus != FLASH_COMPLETE) /* If erase operation was failed, a Flash error code is returned */
	{
		return FlashStatus;
	}
	FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, VALID_PAGE); /* Set Page0 as valid page: Write VALID_PAGE at Page0 base address */
	if (FlashStatus != FLASH_COMPLETE) /* If program operation was failed, a Flash error code is returned */
	{
		return FlashStatus;
	}
	FlashStatus = FLASH_ErasePage(PAGE1_BASE_ADDRESS); /* Erase Page1 */
	return FlashStatus; /* Return Page1 erase operation status */
}
static uint16_t EE_FindValidPage(uint8_t Operation)
{
	uint16_t PageStatus0 = 6, PageStatus1 = 6;
	PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS); /* Get Page0 actual status */
	PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS); /* Get Page1 actual status */
	switch (Operation) /* Write or read operation */
	{
	case WRITE_IN_VALID_PAGE:   /* ---- Write operation ---- */
		if (PageStatus1 == VALID_PAGE)
		{
			if (PageStatus0 == RECEIVE_DATA) return PAGE0; /* Page0 receiving data */ /* Page0 valid */
			else return PAGE1; /* Page1 valid */
		}
		else if (PageStatus0 == VALID_PAGE)
		{
			if (PageStatus1 == RECEIVE_DATA) return PAGE1; /* Page1 receiving data */ /* Page1 valid */
			else return PAGE0; /* Page0 valid */
		}
		else return NO_VALID_PAGE;   /* No valid Page */
    case READ_FROM_VALID_PAGE:  /* ---- Read operation ---- */
    	if (PageStatus0 == VALID_PAGE) return PAGE0;           /* Page0 valid */
    	else if (PageStatus1 == VALID_PAGE) return PAGE1;           /* Page1 valid */
    	else return NO_VALID_PAGE ;  /* No valid Page */
	default:
		return PAGE0; /* Page0 valid */
  }
}
static uint16_t EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data)
{
	FLASH_Status FlashStatus = FLASH_COMPLETE;
	uint16_t ValidPage = PAGE0;
	uint32_t Address = 0x08010000, PageEndAddress = 0x080107FF;

	ValidPage = EE_FindValidPage(WRITE_IN_VALID_PAGE); /* Get valid Page for write operation */
	if (ValidPage == NO_VALID_PAGE) /* Check if there is no valid page */
	{
		return  NO_VALID_PAGE;
	}
	Address = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * PAGE_SIZE)); /* Get the valid Page start Address */
	PageEndAddress = (uint32_t)((EEPROM_START_ADDRESS - 2) + (uint32_t)((1 + ValidPage) * PAGE_SIZE)); /* Get the valid Page end Address */
	while (Address < PageEndAddress) /* Check each active page address starting from begining */
	{
		if ((*(__IO uint32_t*)Address) == 0xFFFFFFFF) /* Verify if Address and Address+2 contents are 0xFFFFFFFF */
		{
			FlashStatus = FLASH_ProgramHalfWord(Address, Data); /* Set variable data */
			if (FlashStatus != FLASH_COMPLETE) /* If program operation was failed, a Flash error code is returned */
			{
				return FlashStatus;
			}
			FlashStatus = FLASH_ProgramHalfWord(Address + 2, VirtAddress); /* Set variable virtual address */
			return FlashStatus; /* Return program operation status */
		}
		else
		{
			Address = Address + 4; /* Next address location */
		}
	}
	return PAGE_FULL; /* Return PAGE_FULL in case the valid page is full */
}
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint16_t Data)
{
	FLASH_Status FlashStatus = FLASH_COMPLETE;
	uint32_t NewPageAddress = 0x080103FF, OldPageAddress = 0x08010000;
	uint16_t ValidPage = PAGE0, VarIdx = 0;
	uint16_t EepromStatus = 0, ReadStatus = 0;

	ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE); /* Get active Page for read operation */
	if (ValidPage == PAGE1)       /* Page1 valid */
	{
		NewPageAddress = PAGE0_BASE_ADDRESS; /* New page address where variable will be moved to */
		OldPageAddress = PAGE1_BASE_ADDRESS; /* Old page address where variable will be taken from */
	}
	else if (ValidPage == PAGE0)  /* Page0 valid */
	{
		NewPageAddress = PAGE1_BASE_ADDRESS; /* New page address where variable will be moved to */
		OldPageAddress = PAGE0_BASE_ADDRESS; /* Old page address where variable will be taken from */
	}
	else
	{
		return NO_VALID_PAGE;       /* No valid Page */
	}
	FlashStatus = FLASH_ProgramHalfWord(NewPageAddress, RECEIVE_DATA); /* Set the new Page status to RECEIVE_DATA status */
	if (FlashStatus != FLASH_COMPLETE) /* If program operation was failed, a Flash error code is returned */
	{
		return FlashStatus;
	}
	EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddress, Data); /* Write the variable passed as parameter in the new active page */
	if (EepromStatus != FLASH_COMPLETE) /* If program operation was failed, a Flash error code is returned */
	{
		return EepromStatus;
	}
	for (VarIdx = 0; VarIdx < NB_OF_VAR; VarIdx++) /* Transfer process: transfer variables from old to the new active page */
	{
		if (VirtAddVarTab[VarIdx] != VirtAddress)  /* Check each variable except the one passed as parameter */
		{
			ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar); /* Read the other last variable updates */
			if (ReadStatus != 0x1) /* In case variable corresponding to the virtual address was found */
			{
				EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar); /* Transfer the variable to the new active page */
				if (EepromStatus != FLASH_COMPLETE)  /* If program operation was failed, a Flash error code is returned */
				{
					return EepromStatus;
				}
			}
		}
	}
	FlashStatus = FLASH_ErasePage(OldPageAddress); /* Erase the old Page: Set old Page status to ERASED status */
	if (FlashStatus != FLASH_COMPLETE) /* If erase operation was failed, a Flash error code is returned */
	{
		return FlashStatus;
	}
	FlashStatus = FLASH_ProgramHalfWord(NewPageAddress, VALID_PAGE); /* Set new Page status to VALID_PAGE status */
	if (FlashStatus != FLASH_COMPLETE) /* If program operation was failed, a Flash error code is returned */
	{
		return FlashStatus;
	}
	return FlashStatus; /* Return last operation flash status */
}

void FLASH_Unlock(void)
{
	if((FLASH->CR & FLASH_CR_LOCK) != 0)
	{
		FLASH->KEYR = FLASH_KEY1;
		FLASH->KEYR = FLASH_KEY2;
	}
}
FLASH_Status FLASH_ErasePage(uint32_t Page_Address)//
{
	FLASH_Status status = FLASH_COMPLETE;
	status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
	if(status == FLASH_COMPLETE)
	{
		FLASH->CR |= FLASH_CR_PER;
		FLASH->AR  = Page_Address;
		FLASH->CR |= FLASH_CR_STRT;
		status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
		FLASH->CR &= ~FLASH_CR_PER;
	}
	return status;
}
FLASH_Status FLASH_ProgramHalfWord(uint32_t Address, uint16_t Data)//
{
	FLASH_Status status = FLASH_COMPLETE;
	status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
	if(status == FLASH_COMPLETE)
	{
		FLASH->CR |= FLASH_CR_PG;
		*(__IO uint16_t*)Address = Data;
		status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
		FLASH->CR &= ~FLASH_CR_PG;
	}
	return status;
}
FLASH_Status FLASH_GetStatus(void)
{
	FLASH_Status FLASHstatus = FLASH_COMPLETE;
	if((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY) FLASHstatus = FLASH_BUSY;
	else if((FLASH->SR & (uint32_t)FLASH_FLAG_WRPERR)!= (uint32_t)0x00) FLASHstatus = FLASH_ERROR_WRP;
	else if((FLASH->SR & (uint32_t)(FLASH_SR_PGERR)) != (uint32_t)0x00) FLASHstatus = FLASH_ERROR_PROGRAM;
	else FLASHstatus = FLASH_COMPLETE;
	return FLASHstatus;
}
FLASH_Status FLASH_WaitForLastOperation(uint32_t Timeout)//
{
	FLASH_Status status = FLASH_COMPLETE;
	status = FLASH_GetStatus();
	while((status == FLASH_BUSY) && (Timeout != 0x00))
	{
		status = FLASH_GetStatus();
		Timeout--;
	}
	if(Timeout == 0x00 ) status = FLASH_TIMEOUT;
	return status;
}

