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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
#include "dma.h"
#include "usart1.h"
#include "ring_buffer.h"
#include "frame_parser.h"

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
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

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
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */

  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_DMA(&huart1, rx_buf, 256);          //���� DMA ���գ����Ӵ��ڣ�USART1���� RX
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);          //ʹ�ܿ����ж�
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	rb_init(&rx_rb);
	parser_init(&my_parser);
	
//	
//	/* === 7.16 Ring Buffer 纯 C 验证 === */
//        uint8_t tx_buf[128];
//        uint16_t tx_len = 0;
//        uint8_t out;

//        // 测试1：基本读写
//        uint8_t test_data[] = "HelloRingBuffer_ABCDEFG_12345";
//        for (int i = 0; i < (int)(sizeof(test_data) - 1); i++) {
//                rb_put(&rx_rb, test_data[i]);
//        }
//        tx_len = 0;
//        while (rb_get(&rx_rb, &out) && tx_len < 128) {
//                tx_buf[tx_len++] = out;
//        }
//        HAL_UART_Transmit(&huart1, (uint8_t*)"[RB_TEST1] ", 11, 100);
//        HAL_UART_Transmit(&huart1, tx_buf, tx_len, 100);
//        HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 100);

//        // 测试2：环绕（wrap around）
//        rb_reset(&rx_rb);
//        for (int i = 0; i < 250; i++) {
//                rb_put(&rx_rb, 'A');
//        }
//        for (int i = 0; i < 250; i++) {
//                rb_get(&rx_rb, &out);
//        }
//        rb_put(&rx_rb, 'X');
//        rb_put(&rx_rb, 'Y');
//        uint8_t out1, out2;
//        rb_get(&rx_rb, &out1);
//        rb_get(&rx_rb, &out2);
//        HAL_UART_Transmit(&huart1, (uint8_t*)"[RB_TEST2] Wrap: ", 17, 100);
//        HAL_UART_Transmit(&huart1, &out1, 1, 100);
//        HAL_UART_Transmit(&huart1, (uint8_t*)" ", 1, 100);
//        HAL_UART_Transmit(&huart1, &out2, 1, 100);
//        HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 100);

//        // 测试3：满缓冲拒绝写入
//        rb_reset(&rx_rb);
//        int ok_count = 0;
//        for (int i = 0; i < 300; i++) {
//                if (rb_put(&rx_rb, 'Z')) ok_count++;
//        }
//        char full_msg[64];
//        int full_len = snprintf(full_msg, sizeof(full_msg),
//                "[RB_TEST3] Full buffer: wrote %d (expected 255)\r\n", ok_count);
//        HAL_UART_Transmit(&huart1, (uint8_t*)full_msg, full_len, 100);
//        /* === 7.16 验证结束 === */
//	
	
	
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
            if (parser_feed(&my_parser, byte) == true)
            {
                /* ---------- 成功拆帧，打印解析结果 ---------- */
                char buf[128];
                int len = 0;

                len = sprintf(buf, "\r\n=== Frame ===\r\n");
                HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, 100);

                len = sprintf(buf, "Addr: 0x%02X\r\n", my_parser.addr);
                HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, 100);

                len = sprintf(buf, "Func: 0x%02X\r\n", my_parser.func);
                HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, 100);

                len = sprintf(buf, "Len : %d\r\n", my_parser.len);
                HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, 100);

                HAL_UART_Transmit(&huart1, (uint8_t*)"Data: ", 6, 100);
                for (int i = 0; i < my_parser.len; i++) {
                    len = sprintf(buf, "%02X ", my_parser.data[i]);
                    HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, 100);
                }
                HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 100);

                len = sprintf(buf, "CRC : 0x%02X%02X\r\n", my_parser.crc_high, my_parser.crc_low);
                HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, 100);

                HAL_UART_Transmit(&huart1, (uint8_t*)"=============\r\n", 15, 100);
                /* ---------- 打印结束 ---------- */
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
