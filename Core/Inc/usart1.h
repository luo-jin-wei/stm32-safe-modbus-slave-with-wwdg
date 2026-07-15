#ifndef USART1_H
#define USART1_H

#include <stdio.h>
#include "main.h"
int fputc(int ch, FILE *f);
void MX_USART1_UART_Init(void);
extern UART_HandleTypeDef huart1;


#endif