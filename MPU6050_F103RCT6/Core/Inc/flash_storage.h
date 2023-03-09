/*
 * flash_storage.h
 *
 *  Created on: Dec 20, 2022
 *      Author: Teeho
 */

#ifndef INC_FLASH_STORAGE_H_
#define INC_FLASH_STORAGE_H_

#include "main.h"

#define startAddressRFC 0x0801F810

#ifdef __cplusplus
extern "C" {
#endif

void Flash_Assign_Param(uint16_t *min_RFC, uint16_t *max_RFC, uint16_t min_RFC_default, uint16_t max_RFC_default);
void Flash_Soft_SetOffset(int RFC_min, int RFC_max);
void Flash_Soft_GetOffset(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_FLASH_STORAGE_H_ */
