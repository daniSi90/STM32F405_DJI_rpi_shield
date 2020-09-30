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
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bno080.h"
//#include "Quaternion.h"
#include "string.h"

#include "ubx_gnss.h"

#include "TR_One_HAL.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// TerraBee One Sensor
TRONE_Str sens[2];
TERAONE_Result res = 0;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
typedef struct stm_rpi{
	int16_t tof_sens[4];
	int16_t bno_sens;
	CGNSS gnss_sensor;
	uint8_t pwm_val;
}stm_rpi;

stm_rpi spi_data;
uint32_t pack_sz;
//stm_rpi spi_data = {.tof_sens[0] = 500, .tof_sens[1] = 600, .tof_sens[2] = 700, .tof_sens[3] = 800, .rtk_sens = 100};

uint8_t pin_state = 0; //for testing only, delete after


__IO uint32_t uwDutyCycle1 = 0;
__IO uint16_t uwDutyCycle2 = 0;
__IO uint32_t uwDutyCyclePre1 = 0;
__IO uint16_t uwDutyCyclePre2 = 0;
__IO uint32_t uwDutyCycleCur1 = 0;
__IO uint16_t uwDutyCycleCur2 = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_SPI_TxCpltCallback (SPI_HandleTypeDef *hspi){
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart);
}
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
  HAL_Delay(5000);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  /* ODKOMENTIRAJ
  bno080_Initialization();  // READ sensor in external interrupt  - void EXTI9_5_IRQHandler(void)
  bno080_enableRotationVector(19000); //enable rotation vector at 200Hz
  HAL_Delay(20);
  bno080_start_IT();
  ODKOMENTIRAJ */

  // RTK
  //sensorRTK = copy_struct(); // Tukaj se nahajajo vsi podatki
  HAL_UART_Receive_DMA(&huart2, &spi_data.gnss_sensor.rx_byte, 1);

  HAL_Delay(5000);

  /* TerraBeeOne */
  sens[0].i2cHandle = &hi2c1;
  sens[0].Address = 0x40 << 1;  // default 0x30 << 1;
  res = TrOne_WhoAmI(&sens[0]);

  // Capture PWM Duty Cycle
  if(HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1) != HAL_OK)
  {
	  //Error_Handler();
  }

  if(HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_4) != HAL_OK)
  {
	  //Error_Handler();
  }

  pack_sz = sizeof(spi_data);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	 TrOne_ReadDist(&sens[0]);
	 spi_data.tof_sens[0] = sens[0].distance;
	 spi_data.gnss_sensor.vel.velocity = sqrt (spi_data.gnss_sensor.vel.N*spi_data.gnss_sensor.vel.N + spi_data.gnss_sensor.vel.E*spi_data.gnss_sensor.vel.E + spi_data.gnss_sensor.vel.E*spi_data.gnss_sensor.vel.E);


	 //spi_data.gnss_sensor.vel.velocity = sqrt (spi_data.gnss_sensor.vel.N*spi_data.gnss_sensor.vel.N + spi_data.gnss_sensor.vel.E*spi_data.gnss_sensor.vel.E + spi_data.gnss_sensor.vel.E*spi_data.gnss_sensor.vel.E);

	 HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)&spi_data, pack_sz);

	 HAL_GPIO_WritePin(RPI_INT_GPIO_Port, RPI_INT_Pin, 1);
	 HAL_Delay(50);
	 HAL_GPIO_WritePin(RPI_INT_GPIO_Port, RPI_INT_Pin, 0);
	 HAL_Delay(50);

	 //pin_state = HAL_GPIO_ReadPin (RPI_INT_GPIO_Port, RPI_INT_Pin);

	 spi_data.bno_sens += 3;
	 if (spi_data.bno_sens > 50) {spi_data.bno_sens = -50;}

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
  RCC_OscInitStruct.PLL.PLLM = 6;
  RCC_OscInitStruct.PLL.PLLN = 168;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart2)		//GNSS
	{
		ubx_handleGNSS(&spi_data.gnss_sensor);
	}
}


/**
  * @brief  Input Capture callback in non blocking mode
  * @param  htim: TIM IC handle
  * @retval None
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
	  uwDutyCycleCur1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
	  uwDutyCycle1 = uwDutyCycleCur1 - uwDutyCyclePre1;
	  uwDutyCyclePre1 = uwDutyCycleCur1;

	  // 16800 - Number of counts for 1ms (OFF), 33600 Number of counts for 2ms (ON)
	  if(uwDutyCycle1 == 33600){
		  spi_data.pwm_val |= 0x02;
	  }else if(uwDutyCycle1 == 16800){
		  spi_data.pwm_val &= 0xFD;
	  }
  }else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4){
	  uwDutyCycleCur2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
	  uwDutyCycle2 = uwDutyCycleCur2 - uwDutyCyclePre2;
	  uwDutyCyclePre2 = uwDutyCycleCur2;
	  if(uwDutyCycle2 == 33600){
		  spi_data.pwm_val |= 0x01;
	  }else if(uwDutyCycle2 == 16800){
		  spi_data.pwm_val &= 0xFE;
	  }
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
  NVIC_SystemReset();
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
