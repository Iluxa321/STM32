#ifndef _UART_
#define _UART_

#include "RTE_Components.h"
#include CMSIS_device_header
#include <stdio.h>
#include "arm_math.h"

typedef union{
	uint8_t b[4];
	float32_t f;
}Matlab_data;


void UsartInit(void);
void uartSendByte(USART_TypeDef *USART, char data);
void sendToMatlab(Matlab_data data);

#endif
