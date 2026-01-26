/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.c
  * @brief   This file provides code for the configuration
  *          of the SPI instances.
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
/* Includes ------------------------------------------------------------------*/
#include "spi.h"

/* USER CODE BEGIN 0 */
#define SELECT() HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET) // CS low
#define DESELECT() HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET) // CS high

#define COMMAND() HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET) // DC=0
#define DATA() HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET)	  // DC=1

#define LCD_RST_L() HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET)
#define LCD_RST_H() HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET)

#define LCD_BL_ON() HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET)
#define LCD_BL_OFF() HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET)
/* USER CODE END 0 */

SPI_HandleTypeDef hspi3;

/* SPI3 init function */
void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 0x0;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi3.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi3.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi3.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi3.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi3.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi3.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi3.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi3.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi3.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(spiHandle->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI3_MspInit 0 */

  /* USER CODE END SPI3_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI3;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* SPI3 clock enable */
    __HAL_RCC_SPI3_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**SPI3 GPIO Configuration
    PC10     ------> SPI3_SCK
    PC11     ------> SPI3_MISO
    PC12     ------> SPI3_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI3_MspInit 1 */

  /* USER CODE END SPI3_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI3_MspDeInit 0 */

  /* USER CODE END SPI3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI3_CLK_DISABLE();

    /**SPI3 GPIO Configuration
    PC10     ------> SPI3_SCK
    PC11     ------> SPI3_MISO
    PC12     ------> SPI3_MOSI
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12);

  /* USER CODE BEGIN SPI3_MspDeInit 1 */

  /* USER CODE END SPI3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

static inline void SPI_transmit(const uint8_t *data, uint16_t length)
{
	HAL_SPI_Transmit(&hspi3, (uint8_t *)data, length, HAL_MAX_DELAY);
}

// ---- ILI write helpers ----
static inline void ILI9844_write_command(uint8_t cmd)
{
	SELECT();
	COMMAND();
	SPI_transmit(&cmd, 1);
	DESELECT();
}

static inline void ILI9844_write_data8(uint8_t data)
{
	SELECT();
	DATA();
	SPI_transmit(&data, 1);
	DESELECT();
}



// ---- init reset sequence ----
static inline void LCD_hw_reset(void)
{
	LCD_BL_ON();
	LCD_RST_H();
	HAL_Delay(10);
	LCD_RST_L();
	HAL_Delay(100);

	LCD_RST_H();
	HAL_Delay(200);
}

void ILI9844_SPI_init(void)
{

	// В этом примере держим CS активным постоянно (как в найденном коде)

	LCD_hw_reset();
	// adjust control 3
	ILI9844_write_command(ILI9844_SOFTWARE_RESET); // ILI9844_SOFTWARE_RESET
	HAL_Delay(10);

	// sleep out
	ILI9844_write_command(ILI9844_SLEEP_OUT); // ILI9844_SLEEP_OUT
	HAL_Delay(120);

	// memory access control (MADCTL)
	ILI9844_write_command(ILI9844_MEMORY_ACCESS_CONTROL);
	ILI9844_write_data8(0x08);

	// frame rate control 1
	//	    ILI9844_write_command(0xB1);          // ILI9844_FRAME_RATE_CONTROL_1
	//	    ILI9844_write_data8(0x00);
	//	    ILI9844_write_data8(0x1B);

	//	     display function control
	ILI9844_write_command(ILI9844_DISPLAY_FUNCTION_CONTROL); // ILI9844_DISPLAY_FUNCTION_CONTROL
	ILI9844_write_data8(0x30);
	ILI9844_write_data8(0x22);
	ILI9844_write_data8(0x3B);

	//	    ILI9844_write_command(0xB6);
	//	    ILI9844_write_data8(0x30);   // BYPASS=0, RCM=0(DE), RM=1(RGB->GRAM), DM=1(DOTCLK)
	//	    ILI9844_write_data8(0x02);   // как в даташите
	//	    ILI9844_write_data8(0x3B);   // как в даташите для 320x480

	// pixel format = 18-bit (0x66)
	ILI9844_write_command(ILI9844_PIXEL_FORMAT_SET); // ILI9844_PIXEL_FORMAT_SET
	ILI9844_write_data8(0x55);

	ILI9844_write_command(ILI9488_INVON); // Invert
	HAL_Delay(10);

	// RGB interface control
	ILI9844_write_command(ILI9844_RGB_INTERFACE_CONTROL); // ILI9844_RGB_INTERFACE_CONTROL
	ILI9844_write_data8(0x00);

	// display on
	ILI9844_write_command(ILI9844_DISPLAY_ON); // ILI9844_DISPLAY_ON
	HAL_Delay(120);

	// interface control (F6) - "select RGB interface" как в примере
	//	    ILI9844_write_command(ILI9844_INTERFACE_CONTROL);          // ILI9844_INTERFACE_CONTROL
	//	    ILI9844_write_data8(0x01);
	//	    ILI9844_write_data8(0x00);
	//	    ILI9844_write_data8(0x06);
}
/* USER CODE END 1 */
