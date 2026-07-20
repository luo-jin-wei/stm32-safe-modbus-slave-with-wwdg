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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
#include "ring_buffer.h"
#include "frame_parser.h"
#include "crc16.h" 
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
uint8_t rx_buf[256];           
volatile uint8_t rx_idle_flag = 0;      //软件变量
FrameParser_t my_parser;
ring_buffer_t rx_rb;
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
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_DMA(&huart1, rx_buf, 256);         
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);         
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	rb_init(&rx_rb);
	parser_init(&my_parser);
	HAL_TIM_Base_Start_IT(&htim2); 


	D1_OFF;
	D2_OFF;
	D3_OFF;
	D4_OFF;

	
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		  if (rx_idle_flag == 1)
    {
        rx_idle_flag = 0;                 //清除标志，如果出现下一帧，
        uint8_t byte;

        /* 从环形缓冲区中取出所有字节，逐个喂给状态机 */
        while (rb_get(&rx_rb, &byte) == 1)
        {
						/* 喂一个字节给状态机，如果返回 true，说明已拆出一帧完整数据 */
//						if (parser_feed(&my_parser, byte) == true)
//						{
//							/* ---------- CRC 校验 ---------- */
//							// 1. 把收到的 CRC 组合成一个 uint16_t
//							//    注意：你的 parser 里 crc_low 是先收到的，crc_high 是后收到的
//							uint16_t rx_crc = ((uint16_t)my_parser.crc_high << 8) | my_parser.crc_low;

//							// 2. 计算本帧的 CRC（范围：addr + func + len + data）
//							uint16_t calc_crc = crc16_modbus_frame(my_parser.addr, my_parser.func, my_parser.len, my_parser.data);

//							// 3. 比对
//							if (calc_crc == rx_crc)
//							{
//									/* ---------- CRC 正确，打印帧内容 ---------- */
//									char buf[128];
//									int len = 0;

//									len = sprintf(buf, "\r\n=== Frame ===\r\n");
//									HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, 100);

//									len = sprintf(buf, "Addr: 0x%02X\r\n", my_parser.addr);
//									HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, 100);

//									len = sprintf(buf, "Func: 0x%02X\r\n", my_parser.func);
//									HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, 100);

//									len = sprintf(buf, "Len : %d\r\n", my_parser.len);
//									HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, 100);

//									HAL_UART_Transmit(&huart1, (uint8_t*)"Data: ", 6, 100);
//									for (int i = 0; i < my_parser.len; i++) {
//											len = sprintf(buf, "%02X ", my_parser.data[i]);
//											HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, 100);
//									}
//									HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 100);

//									len = sprintf(buf, "CRC : 0x%02X%02X\r\n", my_parser.crc_high, my_parser.crc_low);
//									HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, 100);

//									HAL_UART_Transmit(&huart1, (uint8_t*)"=============\r\n", 15, 100);
//							}
//							else
//							{
//									/* ---------- CRC 错误，丢弃并提示 ---------- */
//									HAL_UART_Transmit(&huart1, (uint8_t*)"CRC ERR\r\n", 9, 100);
//							}
//					}
				 if (parser_feed(&my_parser, byte) == true)
  {
      // 把收到的 CRC 组合起来
      uint16_t rx_crc = ((uint16_t)my_parser.crc_high << 8) | my_parser.crc_low;

      // 计算本帧 CRC
      uint16_t calc_crc = crc16_modbus_frame(my_parser.addr, my_parser.func, my_parser.len, my_parser.data);

      /* ========== 加这两行调试打印 ========== */
      char dbg[64];
      int n = sprintf(dbg, "RX_CRC=0x%04X  CALC=0x%04X\r\n", rx_crc, calc_crc);
      HAL_UART_Transmit(&huart1, (uint8_t*)dbg, n, 100);
      /* ======================================= */

      if (calc_crc == rx_crc)
      {
          HAL_UART_Transmit(&huart1, (uint8_t*)"Frame OK\r\n", 10, 100);
      }
      else
      {
          HAL_UART_Transmit(&huart1, (uint8_t*)"CRC ERR\r\n", 9, 100);
      }
  }


        }
    }

    

//  if (rx_idle_flag == 1)                     //基础回显
//  {
//      rx_idle_flag = 0;
//      uint16_t rx_len = 256 - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
//      if (rx_len > 0)
//      {
//          HAL_UART_Transmit(&huart1, rx_buf, rx_len, 100);  // 回显
//      }
//      HAL_UART_AbortReceive(&huart1);
//      HAL_UART_Receive_DMA(&huart1, rx_buf, 256);
//  }
//		
 

		
//		if (rx_idle_flag == 1)                //FIFO成功
//		{
//			rx_idle_flag = 0;
//			uint8_t data;
//			while (rb_get(&rx_rb, &data) == 1)
//			{
//				printf("%c", data);
//			}
//			printf("\r\n"); 
			//HAL_UART_AbortReceive(&huart1);
			//HAL_UART_Receive_DMA(&huart1, rx_buf, 256);
//		}
//	if (rx_idle_flag == 1)
//  {
//      rx_idle_flag = 0;
//      uint8_t data;
//      uint8_t tx_buf[64];
//      uint16_t tx_len = 0;

//      while (rb_get(&rx_rb, &data) == 1 && tx_len < 64)
//      {
//          tx_buf[tx_len++] = data;
//      }

//      if (tx_len > 0)
//      {
//          HAL_UART_Transmit(&huart1, tx_buf, tx_len, 100);
//          HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 100);
//      }

//      HAL_UART_AbortReceive(&huart1);
//      HAL_UART_Receive_DMA(&huart1, rx_buf, 256);
//  }	
		
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
