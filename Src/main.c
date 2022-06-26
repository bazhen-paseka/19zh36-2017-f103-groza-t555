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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

	#include "groza-t55_sm.h"
	#include "ringbuffer_dma_sm.h"
	#include "groza-t55_config.h"

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
  /* USER CODE BEGIN 2 */

	PointStr MyStr0 = {0};
	PointStr MyStr1 = {0};
	PointStr MyStr2 = {0};
	PointStr MyStr3 = {0};
	char DataChar[0xFF];

	Groza_t55_init();

	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET) ;
	HAL_GPIO_WritePin(BUTTON_GND_GPIO_Port, BUTTON_GND_Pin, RESET );
	while (HAL_GPIO_ReadPin(BUTTON_INPUT_GPIO_Port, BUTTON_INPUT_Pin ) == GPIO_PIN_RESET ) {
		Measurement( &MyStr0, 0 );
	}

#if ( FIRST8 == 1 )
	for (int i=0; i<60; i++) {	// wait for router ready
		Measurement( &MyStr0, 0 );
		HAL_Delay(300);
	}

#elif ( NEXT12	== 1)
	for (int i=0; i<70; i++) {	// wait for router ready
		Measurement( &MyStr0, 0 );
		HAL_Delay(300);
	}
#endif

	RingBuffer_DMA_Connect();
	Groza_t55_init();

	//	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_Base_Start_IT(&htim3);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
while (1) {
	//	NRF24L01_Module();
	while (HAL_GPIO_ReadPin(BUTTON_INPUT_GPIO_Port, BUTTON_INPUT_Pin ) == GPIO_PIN_RESET ) {
		if (Get_Flag_1_Sec() == 1) {
			Measurement( &MyStr0, 0);
			Set_Flag_1_Sec(0);
		}
	}
	for (				uint8_t line3=0; line3 < CIRCLE_QNT; line3++ )	{
		for (			uint8_t line2=0; line2 < CIRCLE_QNT; line2++ )	{
			for (		uint8_t line1=0; line1 < CIRCLE_QNT; line1++ )	{
				for (	uint8_t line0=0; line0 < CIRCLE_QNT; line0++ )	{
					while (Get_Flag_1_Sec() == 0) {	/* wait on flag 1 Sec */ }
					Set_Flag_1_Sec(0);
					Measurement( &MyStr0, line0);
				}//for(line0)
				for (uint8_t device = 0; device < DEVICE_QNT; device++) {
					sprintf(DataChar,"  A%d%d%d\t", (int)line3, (int)line2, (int)line1 );
					HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
					MyStr1.point_u32[device][line1] = Calc_Average(MyStr0.point_u32[device], CIRCLE_QNT);
				}
			}//for(line1)
			for (uint8_t device = 0; device < DEVICE_QNT; device++) {
				sprintf(DataChar,"  B%d%d\t", (int)line3, (int)line2 );
				HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
				MyStr2.point_u32[device][line2] = Calc_Average(MyStr1.point_u32[device], CIRCLE_QNT);
			}
		}//for(line2)
		for (uint8_t device = 0; device < DEVICE_QNT; device++) {
			sprintf(DataChar,"  C%d\t", (int)line3 );
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
			MyStr3.point_u32[device][line3] = Calc_Average(MyStr2.point_u32[device], CIRCLE_QNT);
		}
	}//for(line3)

	uint32_t aver_res_u32[DEVICE_QNT];
	for (uint8_t device = 0; device < DEVICE_QNT; device++) {
		sprintf(DataChar,"  D\t" );
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		aver_res_u32[device] = Calc_Average(MyStr3.point_u32[device], CIRCLE_QNT);
	}

	char http_req[0xFF] = { 0 } ;
	sprintf(http_req, "&field1=%d&field2=%d&field3=%d&field4=%d&field5=%d&field6=%d&field7=%d&field8=%d\r\n\r\n",
					(int) aver_res_u32[ 0] ,
					(int) aver_res_u32[ 1] ,
					(int) aver_res_u32[ 2] ,
					(int) aver_res_u32[ 3] ,
					(int) aver_res_u32[ 4] ,
					(int) aver_res_u32[ 5] ,
					(int) aver_res_u32[ 6] ,
					(int) aver_res_u32[ 7] );
	char apiKey_0[] = THINGSPEAK_API_KEY_0 ;
	RingBuffer_DMA_Main(http_req, apiKey_0);
	HAL_Delay(500);

#if ( FIRST8 == 1 )
	sprintf(http_req, "&field1=%d&field2=%d\r\n\r\n",
					(int)((aver_res_u32[12]*4)/10),
					(int)aver_res_u32[14] );
	char apiKey_1[] = THINGSPEAK_API_KEY_1 ;
	RingBuffer_DMA_Main(http_req, apiKey_1);
	HAL_Delay(500);

#elif ( NEXT12	== 1)
	sprintf(http_req, "&field1=%d&field2=%d&field3=%d&field4=%d&field5=%d&field6=%d&field7=%d&field8=%d\r\n\r\n",
					(int) aver_res_u32[ 8] ,
					(int) aver_res_u32[ 9] ,
					(int) aver_res_u32[10] ,
					(int) aver_res_u32[11] ,
					(int) aver_res_u32[12] ,
					(int) aver_res_u32[13] ,
					(int) aver_res_u32[14] ,
					(int) aver_res_u32[15] );
	char apiKey_1[] = THINGSPEAK_API_KEY_1 ;
	RingBuffer_DMA_Main(http_req, apiKey_1);
	HAL_Delay(500);
#endif

	sprintf(http_req, "&field1=%d&field2=%d&field3=%d&field4=%d&field5=%d&field6=%d&field7=%d&field8=%d\r\n\r\n",
					(int) MyStr0.zerone_u32[ 0] ,
					(int) MyStr0.zerone_u32[ 1] ,
					(int) MyStr0.zerone_u32[ 2] ,
					(int) MyStr0.zerone_u32[ 3] ,
					(int) MyStr0.zerone_u32[ 4] ,
					(int) MyStr0.zerone_u32[ 5] ,
					(int) MyStr0.zerone_u32[ 6] ,
					(int) MyStr0.zerone_u32[ 7] ) ;
	char apiKey_2[] = THINGSPEAK_API_KEY_2 ;
	RingBuffer_DMA_Main(http_req, apiKey_2);
	HAL_Delay(500);

	sprintf(http_req, "&field1=%d&field2=%d&field3=%d&field4=%d&field5=%d&field6=%d&field7=%d&field8=%d\r\n\r\n",
					(int) MyStr0.zerone_u32[ 8] ,
					(int) MyStr0.zerone_u32[ 9] ,
					(int) MyStr0.zerone_u32[10] ,
					(int) MyStr0.zerone_u32[11] ,
					(int) MyStr0.zerone_u32[12] ,
					(int) MyStr0.zerone_u32[13] ,
					(int) MyStr0.zerone_u32[14] ,
					(int) MyStr0.zerone_u32[15] ) ;
	char apiKey_3[] = THINGSPEAK_API_KEY_3 ;
	RingBuffer_DMA_Main(http_req, apiKey_3);


	for (int d=0; d < DEVICE_QNT; d++) {
		MyStr0.zerone_u32[d] = 0;
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

