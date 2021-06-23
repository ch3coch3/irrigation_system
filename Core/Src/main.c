/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "semphr.h"
//#include "STM_ENC28_J60.h"

#include "webServer.h"
#include <stdlib.h>
#include <stdarg.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint16_t adc_value;
uint8_t flag = 0;

int waterlevel;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
char writeValue[60];
//char template[800];
char redi[500];
char data[80];

char front[500];
char back[300];

int i;
uint16_t cntBytesTX = 0;
uint32_t sampleTX = 0;
uint8_t S1 = 0;
uint8_t S2 = 0;
uint8_t S3 = 0;

char *request;

uint8_t spiData[2];

// network configuration
uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
uint8_t ip[] = { 192, 168, 0, 116 };
uint16_t port = 80;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void servo_Motor(void *parameters);
void vHandlerTask(void *parameters);
//void water_height(void);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

void frottemp();
void backtemp();

void redirect(void);
//void homePage(void);
void server(void *parameters);

SemaphoreHandle_t xSemaphore;
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
  MX_TIM2_Init();
  MX_ADC1_Init();
  MX_USART3_UART_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim3);
  setup_server(mac, ip, port);
//  homePage();

//  FILE *fp;
//  fp = fopen("/fronttemp.html", "r");
//  if(fp == NULL){
//      printf("Can't open file\n'");
//      return -1;
//  }
//  fread(front, sizeof(char), 500,fp);
//  fclose(fp);
//  fp = fopen("/rediect.html", "r");
//  fread(redi, sizeof(char), 500,fp);
//  fclose(fp);

  redirect();
  frottemp();
  backtemp();

  xSemaphore = xSemaphoreCreateBinary();

  memset(&writeValue,0,sizeof(writeValue));
//  ENC28_Init();
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
//  xTaskCreate(servo_Motor,"servoMotor",1000,NULL,1,NULL);
  xTaskCreate(vHandlerTask,"Handler_Task",1000,NULL,1,NULL);
  xTaskCreate(server,"WebServer",3000,NULL,2,NULL);
  vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	  ENC28_packetSend(42,ARP_req);
//	  HAL_Delay(2000);
//	  water_height();
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 144;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1440;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 72-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_0|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD0 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_0|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void servo_Motor(void *parameters){
	for(;;){
	  TIM2->CCR1 = 35;
	  HAL_Delay(2000);
	  TIM2->CCR1 = 60;
	  HAL_Delay(2000);
	}
}

void vHandlerTask(void *parameters){
//void water_height(void){
	for(;;){
	  char show[80];

	  sprintf(show, "Water is detected! The level : %d  The flag = %d \n\r", waterlevel, flag);
	  HAL_UART_Transmit(&huart3, (uint8_t *)show, strlen(show) ,0xffff);
	  if(flag == 1){
		  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_SET);

	  }
	  else if(flag == 2){
		  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
	  }
	  HAL_Delay(100);

	  xSemaphoreGive(xSemaphore);
//	  HAL_ADC_Start_IT(&hadc1);
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
//	BaseType_t xHigherPriorityTaskWoken;
//	xHigherPriorityTaskWoken = pdFALSE;
	adc_value = HAL_ADC_GetValue(hadc);

	waterlevel = adc_value*40/3000;
	if(waterlevel<18)
	{
		waterlevel = 30;
	}
	else if(waterlevel < 30 && waterlevel >= 18){
		waterlevel = 31;
	}

	waterlevel = waterlevel-30;

	if( adc_value < 2200 && adc_value >10){
		flag = 1;
	}
	else if(adc_value > 2700){
		flag = 2;
	}
//	xSemaphoreGiveFromISR( xSemaphore, &xHigherPriorityTaskWoken );
//	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
int adc=0;
void server(void *parameters){

	for(;;){
		adc += 1;
//

		if(xSemaphoreTake(xSemaphore, portMAX_DELAY)){

			request = serviceRequest();
			if(request != NULL){
				if(strcmp(request,"?control=spray")==0){

					TIM2->CCR1 = 35;
					HAL_Delay(1000);
					TIM2->CCR1 = 55;
					HAL_Delay(1000);

					TIM2->CCR1 = 35;
					HAL_Delay(1000);

					print_text(redi);
				}
				else if(strcmp(request,"?control=add")==0){

					HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, GPIO_PIN_SET);
					HAL_Delay(500);
					HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, GPIO_PIN_RESET);

					print_text(redi);

				}
				else{
					print_text(front);
				}

//				sprintf(data,"<p>Water is detected! The level : %d ,  the flag = %d</p>", adc_value, flag);
				sprintf(data,"<p>The water level : %d ,  the flag = %d</p>", waterlevel, flag);
				print_text(data);
				print_text(back);

				respond_single();

//				HAL_UART_Transmit(&huart3, (uint8_t *)request, strlen(request) ,0xffff);
			}
			else{
				HAL_ADC_Start_IT(&hadc1);
			}

		}
	}
}
void frottemp(){
    // frontHTML template
	strcat(front,"<!DOCTYPE html>");
	strcat(front,"<html lang=\"en\">");
	strcat(front,"<head>");
	strcat(front,"<meta charset=\"UTF-8\">");
	strcat(front,"<meta http-equiv=\"refresh\" content=\"10; url= http://192.168.0.116\">");
//	strcat(front,"<meta http-equiv=\"refresh\" content=\"10\">");
	strcat(front,"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
	strcat(front,"<title>Auto Irrigation System</title>");
	strcat(front,"</head>");
	strcat(front,"<body>");
	strcat(front,"<h1>Auto Irrigation System</h1>");

    strcat(front,"<form action=\"http://192.168.0.116\" method=\"GET\">");
	strcat(front,"<button name = \"control\" type = \"submit\" value = \"add\">add water</button>");
	strcat(front,"<button name = \"control\" type = \"submit\" value = \"spray\">spray water</button>");
}
void backtemp(){
    // back HTML template
	strcat(back,"</form>");
	strcat(back,"</body>");
	strcat(back,"</html>");
}


void redirect(void){
	strcat(redi,"<!DOCTYPE html>");
	strcat(redi,"<html lang=\"en\">");
	strcat(redi,"<head>");
	strcat(redi,"<meta charset=\"UTF-8\">");
	strcat(redi,"<meta http-equiv=\"refresh\" content=\"0; url= http://192.168.0.116\">");
//	strcat(redi,"<meta http-equiv=\"refresh\" content=\"10\">");
	strcat(redi,"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
	strcat(redi,"<title>Auto Irrigation System</title>");
	strcat(redi,"</head>");
	strcat(redi,"<body>");
	strcat(redi,"<h1>Auto Irrigation System</h1>");
	strcat(redi,"<form action=\"http://192.168.0.116\" method=\"GET\">");
	strcat(redi,"<button name = \"control\" type = \"submit\" value = \"add\">add water</button>");
	strcat(redi,"<button name = \"control\" type = \"submit\" value = \"spray\">spray water</button>");
}

//void homePage(void){
//	strcat(template,"<!DOCTYPE html>");
//	strcat(template,"<html lang=\"en\">");
//	strcat(template,"<head>");
//	strcat(template,"<meta charset=\"UTF-8\">");
//	strcat(template,"<meta http-equiv=\"refresh\" content=\"10\">; url= \"http://192.168.0.116\">");
//	strcat(template,"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
//	strcat(template,"<title>Auto Irrigation System</title>");
//	strcat(template,"</head>");
//	strcat(template,"<body>");
//	strcat(template,"<h1>Auto Irrigation System</h1>");
//	strcat(template,"<form action=\"http://192.168.0.116\" method=\"GET\">");
//	strcat(template,"<button name = \"control\" type = \"submit\" value = \"add\">add water</button>");
//	strcat(template,"<button name = \"control\" type = \"submit\" value = \"spray\">spray water</button>");
//
//
//	strcat(template,"</form>");
//	strcat(template,"</body>");
//	strcat(template,"</html>");
//}

/* USER CODE END 4 */

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
