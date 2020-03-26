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

  	  Groza_t55_init();

		#if (TEST_STROBE == 1)
			do {
				TestStrobe();
			}
			while (1);
		#endif

	  RingBuffer_DMA_Connect();

	  //****************************************************************************************

		//	uint8_t dataOut[32];	/* Data received and data for send */
		uint8_t  dataIn[32];
		NRF24L01_Transmit_Status_t transmissionStatus;	/* NRF transmission status */

		#ifdef MASTER
			uint8_t MyAddress[] = { 0, 0, 0, 0, 0x10 };	/* My address */
			uint8_t Tx0Address[] = { 0, 0, 0, 0, 0x21 };	/* Other end address */
			uint8_t Tx1Address[] = { 0, 0, 0, 0, 0x22 };	/* Other end address */
		#endif

		#ifdef SLAVE_21
			uint8_t MyAddress[] = { 0, 0, 0, 0, 0x21 };	/* My address */
			uint8_t TxAddress[] = { 0, 0, 0, 0, 0x10 };	/* Other end address */
		#endif

		#ifdef SLAVE_22
			uint8_t MyAddress[] = { 0, 0, 0, 0, 0x22 };	/* My address */
			uint8_t TxAddress[] = { 0, 0, 0, 0, 0x10 };	/* Other end address */
		#endif
			char DataChar[100];

		NRF24L01_Init(&hspi2, MY_CHANNEL, 32);
		NRF24L01_SetRF(NRF24L01_DataRate_250k, NRF24L01_OutputPower_M6dBm);	/* Set 250kBps data rate and -6dBm output power */
		NRF24L01_SetMyAddress(MyAddress);	/* Set my address, 5 bytes */
		//NRF24L01_SetTxAddress(TxAddress);	/* Set TX address, 5 bytes */

		#ifdef MASTER
			uint32_t sendTime = HAL_GetTick();
			uint8_t errors = 0;
		#endif
			uint32_t lastTime = HAL_GetTick();
			int16_t  waitTime = 0;
			//	uint32_t ID_counter = 0;

	//****************************************************************************************

	  HAL_TIM_Base_Start(&htim3);
  	  HAL_TIM_Base_Start_IT(&htim3);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {

//****************************************************************************************
#ifdef MASTER
	if (HAL_GetTick() - lastTime > 1000) {			/* Every 2 seconds */
		if  ((ID_counter++)%2 == 1){
			DS18b20_ConvertTemp_SkipROM();

			NRF24L01_SetTxAddress(Tx0Address);	/* Set TX address, 5 bytes */
			sprintf(DataChar,"Convert...\r\n%d)dev20\r\n", (int)ID_counter);
		} else {
			int temp_i = DS18b20_Get_Temp_SkipROM();
			float temp_fl = (float)temp_i / 1600.0;
			sprintf(DataChar,"fl: %.03f; t: %d", temp_fl, temp_i/16);
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

			NRF24L01_SetTxAddress(Tx1Address);	/* Set TX address, 5 bytes */
			sprintf(DataChar,"\r\n%d)dev21\r\n", (int)ID_counter);
		}
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		sprintf((char *) dataOut, "Good f103 news  #%d", waitTime++);

		sprintf(DataChar,"TX: %s\r\n", dataOut);
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		/* Transmit data, goes automatically to TX mode */
		NRF24L01_Transmit(dataOut);
		/* Wait for data to be sent */
		do {
			/* Get transmission status */
			transmissionStatus = NRF24L01_GetTransmissionStatus();
		} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);
		sendTime = HAL_GetTick();

		/* Go back to RX mode */
		NRF24L01_PowerUpRx();
		/* Wait received data, wait max 100ms, if time is larger, then data were probably lost */
		while (!NRF24L01_DataReady() && (HAL_GetTick() - sendTime) < 100);
		sprintf(DataChar,"RX: ");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		NRF24L01_GetData(dataIn);				/* Get data from NRF2L01+ */
		sprintf(DataChar,"%s\r\n", dataIn);
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		sprintf(DataChar,"Ping  : %d ms\r\n", (int)(HAL_GetTick() - sendTime));				/* Show ping time */
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		sprintf(DataChar,"Status: ");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		if (transmissionStatus == NRF24L01_Transmit_Status_Ok) {	/* Check transmit status */
			sprintf(DataChar,"OK\r\n");					/* Transmit went OK */
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		} else if (transmissionStatus == NRF24L01_Transmit_Status_Lost) {
			sprintf(DataChar,"LOST\r\n");		/* Message was LOST */
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		} else {
			/* This should never happen */
			sprintf(DataChar,"Sending data: \r\n");
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		}

		errors = 0;
		for (int k = 0; k < sizeof(dataIn) / sizeof(dataIn[0]); k++) {
			errors += (dataIn[k]!=dataOut[k]);
		}
		sprintf(DataChar,"Errors: %d\r\n", errors);
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		sprintf(DataChar,"\r\n");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		lastTime = HAL_GetTick();
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	}
#else
	/* If data is ready on NRF24L01+ */
	if (NRF24L01_DataReady()) {
		//	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		/* Get data from NRF24L01+ */
		NRF24L01_GetData(dataIn);
		HAL_Delay(1);
		sprintf(DataChar,"\r\nRX: %s\r\n", dataIn);
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		/* Send it back, automatically goes to TX mode */
		NRF24L01_SetTxAddress(TxAddress);	/* Set TX address, 5 bytes */
		NRF24L01_Transmit(dataIn);
		/* Wait for data to be sent */
		do {
			/* Wait till sending */
			transmissionStatus = NRF24L01_GetTransmissionStatus();
		} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);
		/* Send done */

		/* Check data & transmit status */
		sprintf(DataChar,"Send it back: ");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		if (transmissionStatus == NRF24L01_Transmit_Status_Ok) {
			/* Transmit went OK */
			sprintf(DataChar,"OK\r\n");
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		} else {
			/* Message was LOST */
			sprintf(DataChar,"ERROR\r\n");
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		}

		/* Go back to RX mode */
		NRF24L01_PowerUpRx();
		waitTime = 0;
		//	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	} else {
		if (HAL_GetTick() - lastTime > 250) {
			if (waitTime == 0) {
			sprintf(DataChar,"\r\nWaiting for data");
				HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
				waitTime++;
			} else if (waitTime > 17) {
			sprintf(DataChar,"\r\n");
				HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
				waitTime = 0;
			} else {
			sprintf(DataChar,".");
				HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
				waitTime++;
			}
			lastTime = HAL_GetTick();
		}
	}
#endif
//****************************************************************************************

	  if (Get_Flag_60_Sec() == 1) {
		  char http_req[200];
		  static uint8_t circle=0;
		  if (circle < CIRCLE_QNT) {
			  Groza_t55_main(circle, http_req );
			  circle++;
		  }
		  if (circle == CIRCLE_QNT) {
			  RingBuffer_DMA_Main(http_req);
			  circle = 0;
		  }
		  Set_Flag_60_Sec(0);
	  }
	  TestStrobe();
//****************************************************************************************
//****************************************************************************************
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

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
