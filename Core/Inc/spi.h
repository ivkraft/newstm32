/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.h
  * @brief   This file contains all the function prototypes for
  *          the spi.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SPI_HandleTypeDef hspi3;

/* USER CODE BEGIN Private defines */
#define ILI9844_SOFTWARE_RESET 0x01
#define ILI9844_SLEEP_OUT 0x11
#define ILI9844_DISPLAY_OFF 0x28
#define ILI9844_DISPLAY_ON 0x29
#define ILI9844_MEMORY_ACCESS_CONTROL 0x36
#define ILI9844_PIXEL_FORMAT_SET 0x3A
#define ILI9844_FRAME_RATE_CONTROL_1 0xB1
#define ILI9844_DISPLAY_FUNCTION_CONTROL 0xB6
#define ILI9844_RGB_INTERFACE_CONTROL 0xB0
#define ILI9844_INTERFACE_CONTROL 0xF6
#define ILI9488_INVOFF 0x20 ///< Display Inversion OFF
#define ILI9488_INVON 0x21	///< Display Inversion ON
/* USER CODE END Private defines */

void MX_SPI3_Init(void);

/* USER CODE BEGIN Prototypes */
void ILI9844_SPI_init(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */

