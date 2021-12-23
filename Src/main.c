/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

	#include "groza-t55_sm.h"
	#include "ringbuffer_dma_sm.h"
	#include "groza-t55_config.h"
	#include "nrf24l01_config.h"
	#include "lcd1602_fc113_sm.h"

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
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */

	PointStr MyStr0 = {0};
	PointStr MyStr1 = {0};
	Groza_t55_init();
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET) ;
	HAL_GPIO_WritePin(BUTTON_GND_GPIO_Port, BUTTON_GND_Pin, RESET );
	while (HAL_GPIO_ReadPin(BUTTON_INPUT_GPIO_Port, BUTTON_INPUT_Pin ) == GPIO_PIN_RESET ) {
		Groza_t55_test();
	}

	//RingBuffer_DMA_Connect();

	//	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_Base_Start_IT(&htim3);

	char DataChar[0xFF];
	#define COUNTER	1
	static uint8_t 	sec_counter = 	COUNTER ;
	static uint8_t 	circle_0		=	0  ;
	static uint8_t 	circle_1		=	0  ;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
while (1) {
//	NRF24L01_Module();

	while (HAL_GPIO_ReadPin(BUTTON_INPUT_GPIO_Port, BUTTON_INPUT_Pin ) == GPIO_PIN_RESET ) {
		if (Get_Flag_1_Sec() == 1) {
			Groza_t55_test();
			sec_counter = COUNTER ;
			Set_Flag_1_Sec(0);
			circle_0 = 0 ;
		}
	}

	if (Get_Flag_1_Sec() == 1) {
		sec_counter--;
		char uart_buffer[0xFF];
		sprintf(uart_buffer," %02d\r", sec_counter );
		HAL_UART_Transmit(&huart1, (uint8_t *)uart_buffer, strlen(uart_buffer), 100);

		if (sec_counter != 0) {
			Groza_t55_test();
		} else {
			sec_counter = COUNTER ;
			if (circle_0 < CIRCLE_QNT) {
				Measurement( &MyStr0, circle_0);
				circle_0++;
			}

			if (circle_0 == CIRCLE_QNT) {
				sprintf(DataChar,"\r\n" );
				HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

				for (uint8_t channel_u8 = 0; channel_u8 < DEVICE_QNT; channel_u8++) {
					sprintf(DataChar,"A%d] %d) ", (int)circle_1, (int)(channel_u8+1) );
					HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

					//aver_res_u32[channel_u8] = Calc_Average(MyStr0.point_u32[channel_u8], CIRCLE_QNT);
					MyStr1.point_u32[channel_u8][circle_1] = Calc_Average(MyStr0.point_u32[channel_u8], CIRCLE_QNT);

					sprintf(DataChar," (%d)\r\n", (int)MyStr1.point_u32[channel_u8][circle_1] );
					HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
				}

				circle_1++;

				if (circle_1 == CIRCLE_QNT) {
					uint32_t aver_res_u32[DEVICE_QNT];
					for (uint8_t channel_u8 = 0; channel_u8 < DEVICE_QNT; channel_u8++) {
						sprintf(DataChar,"B] %d) ", (int)(channel_u8+1) );
						HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

						aver_res_u32[channel_u8] = Calc_Average(MyStr1.point_u32[channel_u8], CIRCLE_QNT);

						sprintf(DataChar," (%d)\r\n", (int)aver_res_u32[channel_u8] );
						HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
					}
					char http_req_1[0xFF] = { 0 } ;
					char http_req_2[0xFF] = { 0 } ;
					sprintf(http_req_1, "&field1=%d&field2=%d&field3=%d&field4=%d&field5=%d&field6=%d&field7=%d&field8=%d\r\n\r\n",
									(int)aver_res_u32[0],
									(int)aver_res_u32[1],
									(int)aver_res_u32[2],
									(int)aver_res_u32[3],
									(int)aver_res_u32[4],
									(int)aver_res_u32[5],
									(int)aver_res_u32[6],
									(int)aver_res_u32[7] );
					sprintf(http_req_2, "&field1=%d&field2=%d\r\n\r\n",
									(int)aver_res_u32[8],
									(int)aver_res_u32[9] );
					sprintf(DataChar,"\r\n" );
					HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100) ;
					//RingBuffer_DMA_Main( http_req_1, http_req_2 );
					circle_1 = 0;
				}
				circle_0 = 0;
			}
		}
		Set_Flag_1_Sec(0);
	}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

