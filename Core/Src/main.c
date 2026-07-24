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
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
#include "ring_buffer.h"       //FIFO
#include "frame_parser.h"      // 帧解析状态机
#include "crc16.h"             // CRC16-Modbus 计算
#include "sht30.h"
#include "modbus_slave.h"
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
uint8_t rx_buf[256];                    //DMA 接收缓冲区   
volatile uint8_t rx_idle_flag = 0;      //空闲中断触发标志
FrameParser_t my_parser;                //帧解析器,存储当前正在组装的一帧的临时数据
ring_buffer_t rx_rb;                    //FIFO中缓存DMA的元素
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_USART1_UART_Init(void);

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
  MX_TIM2_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_DMA(&huart1, rx_buf, 256); // 启动 DMA 接收，缓冲区 rx_buf
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE); //使能 USART1 空闲中断，检测一帧数据接收完毕        
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);    //中断优先级
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	rb_init(&rx_rb);                       // 初始化FIFO
	parser_init(&my_parser);               // 初始化帧解析器
	HAL_TIM_Base_Start_IT(&htim2);         //启动 TIM2 定时器中断，接收数据时使能


	D1_OFF;
	D2_OFF;
	D3_OFF;
	D4_OFF;

	float temp = 0.0f, hum = 0.0f;
	uint8_t ret;

	ret = sht30_init();
	printf("SHT30 init: %d\r\n", ret);

	ret = sht30_read_single_shot(&temp, &hum);
	if (ret == 0) {
			printf("Temp: %.2f°C, Hum: %.2f%%\r\n", temp, hum);
	} else {
			printf("SHT30 read error: %d\r\n", ret);
	}
	
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		
		if (rx_idle_flag == 1)
		{
				rx_idle_flag = 0;
				uint8_t byte;
				uint8_t tx_buf[64];
				uint8_t tx_len = 0;

				while (rb_get(&rx_rb, &byte) == 1)
				{
						if (parser_feed(&my_parser, byte) == true)
						{
								// 处理 Modbus 请求，构造响应帧
								modbus_process_request(&my_parser, tx_buf, &tx_len);

								// 如果有响应，发回给 Modbus Poll
								if (tx_len > 0) {
										HAL_UART_Transmit(&huart1, tx_buf, tx_len, 100);
								}
						}
				}
		}
		
		
		static uint32_t last_tick = 0;
  if (HAL_GetTick() - last_tick >= 1000) {
      last_tick = HAL_GetTick();
      float temp, hum;
      if (sht30_read_single_shot(&temp, &hum) == 0) {
          int16_t temp_int = (int16_t)(temp * 10);  // 30.86°C → 308
          holding_regs[0] = (temp_int >> 8) & 0xFF;  // 0x0000 = 温度高字节
          holding_regs[1] = temp_int & 0xFF;         // 0x0001 = 温度低字节
      }
  }
		
		
		
		
		
		
		
		

//		  if (rx_idle_flag == 1)
//    {
//        rx_idle_flag = 0;                 //清除标志，允许接收下一帧
//        uint8_t byte;

////				float temp, hum;
////				if (sht30_read_single_shot(&temp, &hum) == 0)
////				{
////						printf("Temp: %.2f C, Hum: %.2f %%\r\n", temp, hum);
////				}
////				HAL_Delay(1000);  // 每秒读一次
//			
//			
//        // 从环形缓冲区中取出所有字节，逐个喂给状态机
//        while (rb_get(&rx_rb, &byte) == 1)
//        {
//// parser_feed 返回 true 表示已组装完成一帧完整数据（地址+功能码+长度+数据+CRC）
//					if (parser_feed(&my_parser, byte) == true)
//						{
//								// 把收到的 CRC 组合为一个 16 位值
//								uint16_t rx_crc = ((uint16_t)my_parser.crc_high << 8) | my_parser.crc_low;

//								// 计算本帧 CRC
//								uint16_t calc_crc = crc16_modbus_frame(my_parser.addr, my_parser.func, my_parser.len, my_parser.data);

//								//调试打印
//								char dbg[64];
//								int n = sprintf(dbg, "RX_CRC=0x%04X  CALC=0x%04X\r\n", rx_crc, calc_crc);
//								HAL_UART_Transmit(&huart1, (uint8_t*)dbg, n, 100);

////								if (calc_crc == rx_crc)
////								{
////										HAL_UART_Transmit(&huart1, (uint8_t*)"Frame OK\r\n", 10, 100);
////								}
////								else
////								{
////										HAL_UART_Transmit(&huart1, (uint8_t*)"CRC ERR\r\n", 9, 100);
////								}
//						}
//							

//        }
//    }
		
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
