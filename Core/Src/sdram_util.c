/*
 * sdram_util.c
 *
 *  Created on: Jan 24, 2026
 *      Author: ivkra
 */


#include "sdram_util.h"
#include <stdio.h>

// Приватные дефайны для команд
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)



/**
  * @brief Полный цикл запуска SDRAM
  */
void SDRAM_Init_All(SDRAM_HandleTypeDef *hsdram) {
    // 1. Сначала настраиваем MPU, чтобы проц не улетел в HardFault при обращении

    // 2. Последовательность команд (Sequence)
    FMC_SDRAM_CommandTypeDef Command;

    Command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber = 1;
    Command.ModeRegisterDefinition = 0;
    HAL_SDRAM_SendCommand(hsdram, &Command, 0xFFFF);
    HAL_Delay(1);

    Command.CommandMode = FMC_SDRAM_CMD_PALL;
    HAL_SDRAM_SendCommand(hsdram, &Command, 0xFFFF);

    Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command.AutoRefreshNumber = 8;
    HAL_SDRAM_SendCommand(hsdram, &Command, 0xFFFF);

    uint32_t tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1 |
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
                     SDRAM_MODEREG_CAS_LATENCY_3 |
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    Command.ModeRegisterDefinition = tmpmrd;
    HAL_SDRAM_SendCommand(hsdram, &Command, 0xFFFF);

    // 3. Программируем Refresh Rate (для 125МГц)
    HAL_SDRAM_ProgramRefreshRate(hsdram, 956);
}

/**
  * @brief Простой тест записи/чтения
  */
uint8_t SDRAM_Test(void) {
    volatile uint32_t *mem = (uint32_t*)SDRAM_START_ADDR;
    char debug_msg[64];

    *mem = 0xABCDEF12;
    uint32_t read_val = *mem;

    if (read_val == 0xABCDEF12) {
        return 0; // OK
    } else {
        sprintf(debug_msg, "Fail! Wrote: 0xABCDEF12, Read: 0x%08X\r\n", (unsigned int)read_val);
        return 1;
    }
}
