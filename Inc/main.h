/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define STROBE_X_Pin GPIO_PIN_13
#define STROBE_X_GPIO_Port GPIOC
#define STROBE_Y_Pin GPIO_PIN_14
#define STROBE_Y_GPIO_Port GPIOC
#define STROBE_Z_Pin GPIO_PIN_15
#define STROBE_Z_GPIO_Port GPIOC
#define CHANNEL_0_Pin GPIO_PIN_0
#define CHANNEL_0_GPIO_Port GPIOA
#define CHANNEL_0_EXTI_IRQn EXTI0_IRQn
#define LED_Pin GPIO_PIN_1
#define LED_GPIO_Port GPIOA
#define CHANNEL_1_Pin GPIO_PIN_2
#define CHANNEL_1_GPIO_Port GPIOA
#define CHANNEL_1_EXTI_IRQn EXTI2_IRQn
#define CHANNEL_2_Pin GPIO_PIN_3
#define CHANNEL_2_GPIO_Port GPIOA
#define CHANNEL_2_EXTI_IRQn EXTI3_IRQn
#define CHANNEL_4_Pin GPIO_PIN_4
#define CHANNEL_4_GPIO_Port GPIOA
#define CHANNEL_4_EXTI_IRQn EXTI4_IRQn
#define CE_Pin GPIO_PIN_12
#define CE_GPIO_Port GPIOB
#define CSN_Pin GPIO_PIN_8
#define CSN_GPIO_Port GPIOA
#define BUTTON_GND_Pin GPIO_PIN_8
#define BUTTON_GND_GPIO_Port GPIOB
#define BUTTON_INPUT_Pin GPIO_PIN_9
#define BUTTON_INPUT_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
