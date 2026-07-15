#ifndef LED_H
#define LED_H

#include "main.h"


#define D1_GPIO GPIOA
#define D2_GPIO GPIOA
#define D3_GPIO GPIOA
#define D4_GPIO GPIOB

#define D1_PIN GPIO_PIN_11
#define D2_PIN GPIO_PIN_12
#define D3_PIN GPIO_PIN_15
#define D4_PIN GPIO_PIN_3

#define D1_ON HAL_GPIO_WritePin(D1_GPIO,D1_PIN,GPIO_PIN_RESET)
#define D1_OFF HAL_GPIO_WritePin(D1_GPIO,D1_PIN,GPIO_PIN_SET)
#define D2_ON HAL_GPIO_WritePin(D2_GPIO,D2_PIN,GPIO_PIN_RESET)
#define D2_OFF HAL_GPIO_WritePin(D2_GPIO,D2_PIN,GPIO_PIN_SET)
#define D3_ON HAL_GPIO_WritePin(D3_GPIO,D3_PIN,GPIO_PIN_RESET)
#define D3_OFF HAL_GPIO_WritePin(D3_GPIO,D3_PIN,GPIO_PIN_SET)
#define D4_ON HAL_GPIO_WritePin(D4_GPIO,D4_PIN,GPIO_PIN_RESET)
#define D4_OFF HAL_GPIO_WritePin(D4_GPIO,D4_PIN,GPIO_PIN_SET)
enum LED_NUM
{
	D1,
	D2,
	D3,
	D4
};

enum LED_STATUS
{
	ON,
	OFF
};






void MX_GPIO_Init(void);





#endif