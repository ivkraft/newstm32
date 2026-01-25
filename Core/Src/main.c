/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "ltdc.h"
#include "memorymap.h"
#include "quadspi.h"
#include "spi.h"
#include "usb_device.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <string.h>
#include "sdram_util.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SDRAM_BASE 0xC0000000
uint8_t *fb1 = (uint8_t*) (SDRAM_BASE);
uint8_t *fb2 = (uint8_t*) (SDRAM_BASE + 480 * 320 * 2);

#define SELECT()      HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin,  GPIO_PIN_RESET) // CS low
#define DESELECT()    HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin,  GPIO_PIN_SET)   // CS high

#define COMMAND()     HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,  GPIO_PIN_RESET) // DC=0
#define DATA()        HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,  GPIO_PIN_SET)   // DC=1

#define LCD_RST_L()   HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET)
#define LCD_RST_H()   HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET)

#define LCD_BL_ON()   HAL_GPIO_WritePin(LCD_BL_GPIO_Port,  LCD_BL_Pin,  GPIO_PIN_SET)
#define LCD_BL_OFF()  HAL_GPIO_WritePin(LCD_BL_GPIO_Port,  LCD_BL_Pin,  GPIO_PIN_RESET)

static inline void SPI_transmit(const uint8_t *data, uint16_t length) {
	HAL_SPI_Transmit(&hspi3, (uint8_t*) data, length, HAL_MAX_DELAY);
}

// ---- ILI write helpers ----
static inline void ILI_write_command(uint8_t cmd) {
	SELECT();
	COMMAND();
	SPI_transmit(&cmd, 1);
	DESELECT();
}

static inline void ILI_write_data8(uint8_t data) {
	SELECT();
	DATA();
	SPI_transmit(&data, 1);
	DESELECT();
}

static inline void ILI_write_dataN(const uint8_t *data, uint16_t n) {
	DATA();
	SPI_transmit(data, n);
}

// ---- init reset sequence ----
static inline void LCD_hw_reset(void) {
	LCD_BL_ON();
	LCD_RST_H();
	HAL_Delay(10);
	LCD_RST_L();
	HAL_Delay(100);

	LCD_RST_H();
	HAL_Delay(200);
}

// ======= ТВОЯ ИНИЦИАЛИЗАЦИЯ, НО ПОД НАШ SPI =======
void ILI_init_like_example(void) {

	// В этом примере держим CS активным постоянно (как в найденном коде)

	LCD_hw_reset();
	//adjust control 3
	ILI_write_command(0x01);          // ILI9341_SOFTWARE_RESET
	HAL_Delay(10);

	// sleep out
	ILI_write_command(0x11);          // ILI9341_SLEEP_OUT
	HAL_Delay(120);

	// memory access control (MADCTL)
	ILI_write_command(0x36);          // ILI9341_MEMORY_ACCESS_CONTROL

	ILI_write_data8(0x08);

	// frame rate control 1
//	    ILI_write_command(0xB1);          // ILI9341_FRAME_RATE_CONTROL_1
//	    ILI_write_data8(0x00);
//	    ILI_write_data8(0x1B);

//	     display function control
	ILI_write_command(0xB6);          // ILI9341_DISPLAY_FUNCTION_CONTROL
	ILI_write_data8(0x32);
	ILI_write_data8(0x42);
	ILI_write_data8(0x3B);

//	    ILI_write_command(0xB6);
//	    ILI_write_data8(0x30);   // BYPASS=0, RCM=0(DE), RM=1(RGB->GRAM), DM=1(DOTCLK)
//	    ILI_write_data8(0x02);   // как в даташите
//	    ILI_write_data8(0x3B);   // как в даташите для 320x480

	// pixel format = 18-bit (0x66)
	ILI_write_command(0x3A);          // ILI9341_PIXEL_FORMAT_SET
	ILI_write_data8(0x55);

	ILI_write_command(0x21);          // Invert
	HAL_Delay(10);

	// RGB interface control
	ILI_write_command(0xB0);          // ILI9341_RGB_INTERFACE_CONTROL
	ILI_write_data8(0x00);

	// display on
	ILI_write_command(0x29);          // ILI9341_DISPLAY_ON
	HAL_Delay(120);

	// interface control (F6) - "select RGB interface" как в примере
//	    ILI_write_command(0xF6);          // ILI9341_INTERFACE_CONTROL
//	    ILI_write_data8(0x01);
//	    ILI_write_data8(0x00);
//	    ILI_write_data8(0x06);

}
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */
uint8_t USB_Log(char *str) {
	uint8_t result;

	// Проверяем, что USB вообще сконфигурирован хостом
	extern USBD_HandleTypeDef hUsbDeviceFS;
	if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) {
		return 1;
	}

	// Пытаемся отправить данные
	result = CDC_Transmit_FS((uint8_t*) str, strlen(str));

	if (result == USBD_OK) {
		return 0; // Всё ушло в буфер
	} else {
		return 1; // Занято (USBD_BUSY) или ошибка
	}
}

static void fb_fill(uint16_t color) {
	uint16_t *fb = (uint16_t*) SDRAM_BASE; // Используем твой маппинг

	for (uint32_t i = 0; i < (480 * 320); i++) {
		fb[i] = color;
	}

	/* КРИТИЧЕСКИ ВАЖНО ДЛЯ H7: Сброс кэша данных */
	/* Без этого LTDC не увидит обновленные пиксели в SDRAM */
	SCB_CleanDCache_by_Addr((uint32_t*) fb, 480 * 320 * 2);
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MPU Configuration--------------------------------------------------------*/
	MPU_Config();

	/* Enable the CPU Cache */

	/* Enable I-Cache---------------------------------------------------------*/
	SCB_EnableICache();

	/* Enable D-Cache---------------------------------------------------------*/
	SCB_EnableDCache();

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_FMC_Init();
	MX_QUADSPI_Init();
	MX_SPI3_Init();
	MX_USB_DEVICE_Init();
	/* USER CODE BEGIN 2 */
	SDRAM_Init_All(&hsdram1);

	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(50);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(150);
	HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
	ILI_init_like_example();
	MX_LTDC_Init();

	LTDC_LayerCfgTypeDef pLayerCfg = { 0 };

	// Окно отрисовки (весь экран)
	pLayerCfg.WindowX0 = 0;
	pLayerCfg.WindowX1 = 320; // Ширина
	pLayerCfg.WindowY0 = 0;
	pLayerCfg.WindowY1 = 480; // Высота

	pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
	pLayerCfg.Alpha = 255;
	pLayerCfg.Alpha0 = 255; // Обычно фон прозрачный, если слой перекрывает его

	// Правильные факторы смешивания для непрозрачного слоя
	pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
	pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;

	// Твой проверенный адрес в SDRAM!
	pLayerCfg.FBStartAdress = (uint32_t) SDRAM_BASE;

	pLayerCfg.ImageWidth = 320;
	pLayerCfg.ImageHeight = 480;

	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;

	HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0);
	// Применяем настройки
	HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_IMMEDIATE);

	uint8_t ram_status = SDRAM_Test();
//  fb_fill(0x001F);
	// Даем USB пару секунд, чтобы Windows его точно увидела

	if (ram_status == 0) {
		USB_Log(">>> SDRAM STATUS: OK! <<<\n");
	} else {
		USB_Log(">>> SDRAM STATUS: FAIL! <<<\n");
	}
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		fb_fill(0xF800);
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		USB_Log("RED\n");
		HAL_Delay(500);

		fb_fill(0x07E0);
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
		USB_Log("GREEN\n");
		HAL_Delay(500);

		fb_fill(0x001F);
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		USB_Log("BLUE\n");
		HAL_Delay(500);


		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Supply configuration update enable
	 */
	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

	/** Configure the main internal regulator output voltage
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48
			| RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 2;
	RCC_OscInitStruct.PLL.PLLN = 64;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1
			| RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void) {
	MPU_Region_InitTypeDef MPU_InitStruct = { 0 };

	/* Disables the MPU */
	HAL_MPU_Disable();

	/** Initializes and configures the Region and the memory to be protected
	 */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.BaseAddress = 0xC0000000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
	MPU_InitStruct.SubRegionDisable = 0x0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	/* Enables the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
