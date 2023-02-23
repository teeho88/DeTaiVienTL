/*
 * flash_storage.c
 *
 *  Created on: Dec 20, 2022
 *      Author: Teeho
 */
#include "flash_storage.h"
#include "math.h"

uint16_t *minRFC;
uint16_t *maxRFC;
uint16_t minRFC_default;
uint16_t maxRFC_default;

void Flash_Assign_Param(uint16_t *min_RFC, uint16_t *max_RFC, uint16_t min_RFC_default, uint16_t max_RFC_default)
{
	minRFC = min_RFC;
	maxRFC = max_RFC;
	minRFC_default = min_RFC_default;
	maxRFC_default = max_RFC_default;
}

void Flash_Soft_SetOffset(int RFC_min, int RFC_max)
{
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SECTORError;
	EraseInitStruct.Banks = FLASH_BANK_1;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.Sector = FLASH_SECTOR_7;
	EraseInitStruct.NbSectors = 1;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

	HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);

	if(RFC_min >= 0)
	{
		*minRFC = RFC_min;
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, startAddressRFC, *((uint32_t*)minRFC));
	}

	if(RFC_max >= 0)
	{
		*maxRFC = RFC_max;
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, startAddressRFC + 4, *((uint32_t*)maxRFC));
	}

	HAL_FLASH_Lock();
}


void Flash_Soft_GetOffset(void)
{
	float temp;
	temp = *((uint16_t*)((__IO uint32_t *)(startAddressRFC)));
	if (isnan(temp)) *minRFC = minRFC_default;
	else *minRFC = temp;

	temp = *((uint16_t*)((__IO uint32_t *)(startAddressRFC + 4)));
	if (isnan(temp)) *maxRFC = maxRFC_default;
	else *maxRFC = temp;
}



