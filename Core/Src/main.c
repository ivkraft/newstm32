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
#include "quadspi.h"
#include "spi.h"
#include "stm32h7xx_hal.h"
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

uint16_t *frame_buffers[2] = {
    (uint16_t *)(SDRAM_BASE),                                 // FB1
    (uint16_t *)(SDRAM_BASE + (LCD_WIDTH * LCD_HEIGHT * 2))   // FB2
};

uint8_t current_buffer = 0; // Какой буфер сейчас рисуем

int cursor_x = 0;
int cursor_y = 0;

 void draw_char_8x16(int x, int y, char c, uint16_t color, uint16_t *fb) { 
     // Вычисляем адрес начала символа в массиве 
     // c * 16, так как каждый символ занимает 16 байт 
     const uint8_t *glyph = &fnt_wang_8x16[(uint8_t)c * 16]; 

     for (int i = 0; i < 16; i++) { // Проход по строкам (высота 16) 
         uint8_t row_data = glyph[i]; 
          
         for (int j = 0; j < 8; j++) { // Проход по битам (ширина 8) 
             // В VGA шрифтах самый левый пиксель — это старший бит (0x80) 
             if (row_data & (0x80 >> j)) { 
                 // Простейшая проверка границ, чтобы не "повесить" H7 
                 if ((x + j) < LCD_HEIGHT && (y + i) < LCD_WIDTH) { 
                     fb[(y + i) * LCD_HEIGHT + (x + j)] = color; 
                 } 
             } 
         } 
     } 
 }   

int _write(int file, char *ptr, int len) {
    uint16_t *fb = frame_buffers[current_buffer & 1];

    for (int i = 0; i < len; i++) {
        if (ptr[i] == '\n') {
            cursor_x = 0;
            cursor_y += FONT_H;
        } 
        else if (ptr[i] == '\r') {
            cursor_x = 0;
        }
        else {
            draw_char_8x16(cursor_x, cursor_y, ptr[i], 0xFFFF, fb);
            cursor_x += FONT_W;
        }

        // Автоперенос на новую строку по ширине
        if (cursor_x > (LCD_WIDTH - FONT_W)) {
            cursor_x = 0;
            cursor_y += FONT_H;
        }
        
        // Сброс в начало экрана по высоте
        if (cursor_y > (LCD_HEIGHT - FONT_H)) {
            cursor_y = 0;
        }
    }
    return len;
}

void screen_clear(uint16_t color) {
    // Дублируем 16-бит цвет в 32-битную переменную
    uint32_t color32 = color | (color << 16);
    uint16_t *fb = frame_buffers[current_buffer&1];
    uint32_t count = 4800; // (320 * 480 * 2) / 64

    __asm__ __volatile__ (
        "mov r0, %[clr]\n\t"
        "mov r1, %[clr]\n\t"
        "mov r2, %[clr]\n\t"
        "mov r3, %[clr]\n\t"
    "1:\n\t"
        "stm %[fb]!, {r0-r3}\n\t" // 16 байт
        "stm %[fb]!, {r0-r3}\n\t" // 16 байт
        "stm %[fb]!, {r0-r3}\n\t" // 16 байт
        "stm %[fb]!, {r0-r3}\n\t" // 16 байт
        "subs %[cnt], %[cnt], #1\n\t"
        "bne 1b\n\t"
        : [fb] "+r" (fb), [cnt] "+r" (count)
        : [clr] "r" (color32)
        : "r0", "r1", "r2", "r3", "cc", "memory"
    );
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



void draw_rect(int x, int y, int w, int h, uint16_t color) {
    uint16_t *fb = frame_buffers[current_buffer];
    
    for (int i = y; i < y + h; i++) {
        // Если строка вне экрана — пропускаем её
        if (i < 0 || i >= 480) continue; 
        
        for (int j = x; j < x + w; j++) {
            // Если пиксель вне строки — пропускаем его
            if (j < 0 || j >= 320) continue;
            
            fb[i * 320 + j] = color;
        }
    }
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
 // SCB_EnableDCache();

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
	ILI9844_SPI_init();
	MX_LTDC_Init();

	LTDC_LayerCfgTypeDef pLayerCfg = {0};

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
	pLayerCfg.FBStartAdress = (uint32_t)frame_buffers[current_buffer];

	pLayerCfg.ImageWidth = 320;
	pLayerCfg.ImageHeight = 480;

	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;

	HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0);
	// Применяем настройки
	HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_IMMEDIATE);

	//  fb_fill(0x001F);
	// Даем USB пару секунд, чтобы Windows его точно увидела

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
int dx = 1;
int dy = 1;
  int x = 50; 
int y = 50;
int rect_size = 40;
 uint16_t *draw_fb1 = frame_buffers[0];
    memset(draw_fb1, 0, 320 * 480 * 2); 
     uint16_t *draw_fb2 = frame_buffers[1];
    memset(draw_fb2, 0, 320 * 480 * 2); 
uint32_t frame_count = 0;
uint32_t last_time = 0;
uint32_t fps_val = 0;
	while (1)
   /* USER CODE BEGIN 3 */
{
// 2. Внутри цикла while(1)
frame_count++;
if (HAL_GetTick() - last_time >= 1000) {
    fps_val = frame_count;
    frame_count = 0;
    last_time = HAL_GetTick();
}

// 3. Вывод


      uint16_t *draw_fb = frame_buffers[current_buffer & 1];

      //screen_clear(0x0000); // Черный фон
    memset(draw_fb, 0, 320 * 480 * 2); 



    // 3. Рассчитываем новую позицию (общую для логики)
    x += dx;
    y += dy;
    if (x <= 0 || x + rect_size >= 319) dx = -dx;
    if (y <= 0 || y + rect_size >= 479) dy = -dy;

    // 4. РИСУЕМ новый зеленый квадрат
    draw_rect(x, y, rect_size, rect_size, 0x07E0);

cursor_x = 10; cursor_y = 10;

printf("FPS: %u\n", (unsigned int)fps_val);
   


    // Синхронизация и вывод
    
    HAL_LTDC_SetAddress_NoReload(&hltdc, (uint32_t)draw_fb, 0); 
		HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);

		while ((hltdc.Instance->SRCR & LTDC_RELOAD_VERTICAL_BLANKING) != 0) {}    //USB_Log("go");
    current_buffer = !current_buffer;

  // HAL_Delay(30);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

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
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
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
