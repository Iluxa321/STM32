#include "UART.h"

	
void UsartInit(void){
	/////////////////////////////
	// GPIO Init (P8 - RX, P9 - TX)
	/////////////////////////////
	
	// PORTB Enable 
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN; 
	// ��������� GPIO �� �������������� �������
	GPIOB->MODER |= GPIO_MODER_MODER9_1 | GPIO_MODER_MODER8_1;
	GPIOB->MODER &= ~(GPIO_MODER_MODER9_0 | GPIO_MODER_MODER8_0);
	GPIOB->AFR[1] |= (0x07 << GPIO_AFRH_AFRH1_Pos) | (0x07 << GPIO_AFRH_AFRH0_Pos);
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR9_0;
	
	/////////////////////////////
	// USART Init
	/////////////////////////////
	
	// USART3 Enable
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
	// ��������� ����� �����
	USART3->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0); //  1 Start bit, 8 data bits, n stop bits
	// ��������� ���������� �������� ���
	USART3->CR2 &= ~USART_CR2_STOP; // 1 stop bit
	// ��������� baud rate
	USART3->BRR = (SystemCoreClock / 2) / 115200;
	USART3->RQR |= USART_RQR_RXFRQ; // �������� ���� RXNE
	
	//USART3->CR3 |= USART_CR3_DMAT;	// ���������� ������ DMA �� �����������
	USART3->CR3 |= USART_CR3_DMAR;	// ���������� ������ DMA �� ���������
	USART3->CR1 |= USART_CR1_TE;
	USART3->CR1 |= USART_CR1_RE;
	
	//NVIC_EnableIRQ(USART3_IRQn);
	//USART3->CR1 |= USART_CR1_RXNEIE; // interupte enable
	
	USART3->CR1 |= USART_CR1_UE;
}


void uartSendByte(USART_TypeDef *USART, char data){
	while(!(USART->CR1 & USART_CR1_TE));
	USART->TDR = data;
	while(!((USART->CR1 & USART_CR1_TE) && (USART->ISR & USART_ISR_TC)));
}

uint8_t uartByteAvialable(USART_TypeDef *USART){
	return USART->ISR & USART_ISR_RXNE;
}

char uartReadByte(USART_TypeDef *USART){
	return USART->RDR;
}

void sendToMatlab(Matlab_data data){
	for(int i = 0; i < 4; i++)
		uartSendByte(USART3, data.b[i]);
	//uartSendByte(USART3, '\r');
	//uartSendByte(USART3, '\n');
}
