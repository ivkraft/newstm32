/*
 * sdram_util.h
 *
 *  Created on: Jan 24, 2026
 *      Author: ivkra
 */

#ifndef INC_SDRAM_UTIL_H_
#define INC_SDRAM_UTIL_H_

#include "main.h"

// Адрес и размер (256Mb = 32MB)
#define SDRAM_START_ADDR       ((uint32_t)0xC0000000)
#define SDRAM_SIZE             ((uint32_t)0x02000000)

// Прототипы функций
void SDRAM_Init_All(SDRAM_HandleTypeDef *hsdram);
uint8_t SDRAM_Test(void);


#endif /* INC_SDRAM_UTIL_H_ */
