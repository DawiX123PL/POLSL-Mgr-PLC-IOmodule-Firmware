/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
void SystemClock_Config(void);
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CONFIG_1_Pin GPIO_PIN_13
#define CONFIG_1_GPIO_Port GPIOC
#define DQ6_HIGH_Pin GPIO_PIN_14
#define DQ6_HIGH_GPIO_Port GPIOC
#define DQ6_LOW_Pin GPIO_PIN_15
#define DQ6_LOW_GPIO_Port GPIOC
#define DI7_Pin GPIO_PIN_0
#define DI7_GPIO_Port GPIOA
#define DI6_Pin GPIO_PIN_1
#define DI6_GPIO_Port GPIOA
#define DI5_Pin GPIO_PIN_2
#define DI5_GPIO_Port GPIOA
#define DI4_Pin GPIO_PIN_3
#define DI4_GPIO_Port GPIOA
#define DI3_Pin GPIO_PIN_4
#define DI3_GPIO_Port GPIOA
#define DI2_Pin GPIO_PIN_5
#define DI2_GPIO_Port GPIOA
#define DI1_Pin GPIO_PIN_6
#define DI1_GPIO_Port GPIOA
#define DI0_Pin GPIO_PIN_7
#define DI0_GPIO_Port GPIOA
#define V_SENSE_Pin GPIO_PIN_0
#define V_SENSE_GPIO_Port GPIOB
#define CONFIG_1B1_Pin GPIO_PIN_1
#define CONFIG_1B1_GPIO_Port GPIOB
#define DQ4_HIGH_Pin GPIO_PIN_2
#define DQ4_HIGH_GPIO_Port GPIOB
#define DQ4_LOW_Pin GPIO_PIN_10
#define DQ4_LOW_GPIO_Port GPIOB
#define DQ3_HIGH_Pin GPIO_PIN_11
#define DQ3_HIGH_GPIO_Port GPIOB
#define DQ3_LOW_Pin GPIO_PIN_12
#define DQ3_LOW_GPIO_Port GPIOB
#define DQ2_HIGH_Pin GPIO_PIN_13
#define DQ2_HIGH_GPIO_Port GPIOB
#define DQ2_LOW_Pin GPIO_PIN_14
#define DQ2_LOW_GPIO_Port GPIOB
#define DQ1_HIGH_Pin GPIO_PIN_15
#define DQ1_HIGH_GPIO_Port GPIOB
#define DQ1_LOW_Pin GPIO_PIN_8
#define DQ1_LOW_GPIO_Port GPIOA
#define DQ0_HIGH_Pin GPIO_PIN_9
#define DQ0_HIGH_GPIO_Port GPIOA
#define DQ0_LOW_Pin GPIO_PIN_10
#define DQ0_LOW_GPIO_Port GPIOA
#define LED_GREEN_Pin GPIO_PIN_11
#define LED_GREEN_GPIO_Port GPIOA
#define LED_ORANGE_Pin GPIO_PIN_12
#define LED_ORANGE_GPIO_Port GPIOA
#define LED_RED_Pin GPIO_PIN_6
#define LED_RED_GPIO_Port GPIOF
#define DQ5_LOW_Pin GPIO_PIN_6
#define DQ5_LOW_GPIO_Port GPIOB
#define DQ5_HIGH_Pin GPIO_PIN_7
#define DQ5_HIGH_GPIO_Port GPIOB
#define DQ7_LOW_Pin GPIO_PIN_8
#define DQ7_LOW_GPIO_Port GPIOB
#define DQ7_HIGH_Pin GPIO_PIN_9
#define DQ7_HIGH_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
