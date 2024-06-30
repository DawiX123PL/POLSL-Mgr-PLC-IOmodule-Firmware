/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "adc.h"
#include "crc.h"
#include "dma.h"
#include "iwdg.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14
                              |RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void DisablePeripheralModules()
{
  //  Disable interrupts
  __disable_irq();
  // disable all IO
  HAL_GPIO_DeInit(DQ6_HIGH_GPIO_Port, DQ6_HIGH_Pin);
  HAL_GPIO_DeInit(DQ6_LOW_GPIO_Port, DQ6_LOW_Pin);
  HAL_GPIO_DeInit(DQ4_HIGH_GPIO_Port, DQ4_HIGH_Pin);
  HAL_GPIO_DeInit(DQ4_LOW_GPIO_Port, DQ4_LOW_Pin);
  HAL_GPIO_DeInit(DQ3_HIGH_GPIO_Port, DQ3_HIGH_Pin);
  HAL_GPIO_DeInit(DQ3_LOW_GPIO_Port, DQ3_LOW_Pin);
  HAL_GPIO_DeInit(DQ2_HIGH_GPIO_Port, DQ2_HIGH_Pin);
  HAL_GPIO_DeInit(DQ2_LOW_GPIO_Port, DQ2_LOW_Pin);
  HAL_GPIO_DeInit(DQ1_HIGH_GPIO_Port, DQ1_HIGH_Pin);
  HAL_GPIO_DeInit(DQ1_LOW_GPIO_Port, DQ1_LOW_Pin);
  HAL_GPIO_DeInit(DQ0_HIGH_GPIO_Port, DQ0_HIGH_Pin);
  HAL_GPIO_DeInit(DQ0_LOW_GPIO_Port, DQ0_LOW_Pin);
  HAL_GPIO_DeInit(DQ5_LOW_GPIO_Port, DQ5_LOW_Pin);
  HAL_GPIO_DeInit(DQ5_HIGH_GPIO_Port, DQ5_HIGH_Pin);
  HAL_GPIO_DeInit(DQ7_LOW_GPIO_Port, DQ7_LOW_Pin);
  HAL_GPIO_DeInit(DQ7_HIGH_GPIO_Port, DQ7_HIGH_Pin);
  // Disable all modules
  HAL_ADC_DeInit(&hadc);
  HAL_TIM_Base_DeInit(&htim3);
}

static void LedRed(uint8_t enable)
{
  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, enable);
}

static void LedOrange(uint8_t enable)
{
  HAL_GPIO_WritePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin, enable);
}

static void LedGreen(uint8_t enable)
{
  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, enable);
}

static void LedRedToggle()
{
  HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
}

static void LedOrangeToggle()
{
  HAL_GPIO_TogglePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin);
}

static void LedGreenToggle()
{
  HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}

void NoIRQDelay(uint32_t delay)
{
  // value adjusted experimentaly
  const uint32_t loops_per_ms = 5000;
  uint32_t loops = (delay * loops_per_ms);

  while (loops--)
  {
    asm volatile("nop");
  }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  DisablePeripheralModules();
  while (1)
  {
    DisablePeripheralModules();

    LedOrange(0);
    LedGreen(0);

    // blink Red led
    LedRedToggle(0);
    NoIRQDelay(500);
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
  __disable_irq();
  DisablePeripheralModules();
  while (1)
  {
    DisablePeripheralModules();

    LedOrange(0);
    LedGreen(0);

    // blink Red led
    LedRedToggle(0);
    NoIRQDelay(100);
  }
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
