#include "DMA.h"



void DMA_Init(DMA_Channel_TypeDef *Channel, uint32_t perif, uint32_t mem, uint32_t cnt, uint32_t config){
/////////////////////////////////////////////
// Настрока DMA для передачи данных по USART
/////////////////////////////////////////////
	RCC->AHBENR |= RCC_AHBENR_DMA1EN; // разрешаем DMA
	Channel->CPAR = perif; 
	Channel->CMAR = mem;
	Channel->CNDTR = cnt;
	Channel->CCR |= config;
}

void DMA_DeInit(DMA_Channel_TypeDef *Channel){
	Channel->CPAR = 0;
	Channel->CMAR = 0;
	Channel->CNDTR = 0;
	Channel->CCR |= 0;
	if(Channel == DMA1_Channel1)
		DMA1->IFCR |= DMA_IFCR_CGIF1 | DMA_IFCR_CTCIF1 | DMA_IFCR_CHTIF1 | DMA_IFCR_CTEIF1;
	else if(Channel == DMA1_Channel2)
		DMA1->IFCR |= DMA_IFCR_CGIF2 | DMA_IFCR_CTCIF2 | DMA_IFCR_CHTIF2 | DMA_IFCR_CTEIF2;
	else if(Channel == DMA1_Channel3)
		DMA1->IFCR |= DMA_IFCR_CGIF3 | DMA_IFCR_CTCIF3 | DMA_IFCR_CHTIF3 | DMA_IFCR_CTEIF3;
	else if(Channel == DMA1_Channel4)
		DMA1->IFCR |= DMA_IFCR_CGIF4 | DMA_IFCR_CTCIF4 | DMA_IFCR_CHTIF4 | DMA_IFCR_CTEIF4;
	else if(Channel == DMA1_Channel5)
		DMA1->IFCR |= DMA_IFCR_CGIF5 | DMA_IFCR_CTCIF5 | DMA_IFCR_CHTIF5 | DMA_IFCR_CTEIF5;
	else if(Channel == DMA1_Channel6)
		DMA1->IFCR |= DMA_IFCR_CGIF6 | DMA_IFCR_CTCIF6 | DMA_IFCR_CHTIF6 | DMA_IFCR_CTEIF6;
	else if(Channel == DMA1_Channel7)
		DMA1->IFCR |= DMA_IFCR_CGIF7 | DMA_IFCR_CTCIF7 | DMA_IFCR_CHTIF7 | DMA_IFCR_CTEIF7;
}


/*
void DMA1_chanel1_init(void){
	////////////////////////////////////////
	// Настройка DMA для работы с АЦП
	////////////////////////////////////////
	RCC->AHBENR |= RCC_AHBENR_DMA1EN; 	// разрешаем DMA
	DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR; // адресс откуда берем данные
	DMA1_Channel1->CMAR = (uint32_t)&adc_result[0];  // адресс куда кладем данные
	DMA1_Channel1->CNDTR = (sizeof(adc_result) / sizeof(adc_result[0]));
	DMA1_Channel1->CCR |= DMA_CCR_CIRC; // circular mode
	DMA1_Channel1->CCR &= ~DMA_CCR_DIR; // peripheral->memory
	DMA1_Channel1->CCR |= DMA_CCR_MINC; // memory inc
	DMA1_Channel1->CCR &= ~DMA_CCR_PINC; // peripheral not inc
	DMA1_Channel1->CCR |= (DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0); // 16 bit
	
	NVIC_EnableIRQ(DMA1_Channel1_IRQn); // разрешаем глобально прерывания
	DMA1_Channel1->CCR |= DMA_CCR_TCIE; // разрешаем прерывание по окончания передачи
	
	
	DMA1->IFCR |= DMA_IFCR_CTCIF1; // очищаем флаг окончани¤ передачи
	DMA1_Channel1->CCR |= DMA_CCR_EN;
	
}
*/


/*
void DMA_Init(void){
	
/////////////////////////////////////////////
// Настрока DMA для передачи данных по USART
/////////////////////////////////////////////
	RCC->AHBENR |= RCC_AHBENR_DMA1EN; // разрешаем DMA
	DMA1_Channel2->CPAR = (uint32_t)&USART3->TDR; // адресс TX
	DMA1_Channel2->CMAR = (uint32_t)arr;
	DMA1_Channel2->CNDTR = (sizeof(arr) / sizeof(arr[0])) - 1; // \0 передавать не надо
	DMA1_Channel2->CCR |= DMA_CCR_DIR | DMA_CCR_CIRC; // memory->peripheral; circular mode
	DMA1_Channel2->CCR |= DMA_CCR_MINC; // memory inc
	DMA1_Channel2->CCR &= ~DMA_CCR_PINC; // peripheral not inc
	DMA1_Channel2->CCR &= ~(DMA_CCR_MSIZE | DMA_CCR_PSIZE); // 8 bit
	DMA1->IFCR |= DMA_IFCR_CTCIF2; // очищаем флаг окончания передачи
	DMA1_Channel2->CCR |= DMA_CCR_EN;
}
*/