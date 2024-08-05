#include "RCC.h"


uint8_t ClockInit(void){
	uint32_t start_up_cnt = 0;
	
	//////////////////////////////////////////////////
	// Включение внешнего кварца на 8 МГц
	//////////////////////////////////////////////////
	
	RCC->CR |= RCC_CR_HSEON; // включаем HSE
	while(!(RCC->CR & RCC_CR_HSERDY)){ // ждем пока выставится флаг ready для HSE
		if(start_up_cnt > 0x10000){ // если прошло много времени то отключаем настройки
			RCC->CR &= ~RCC_CR_HSEON; // отключаем HSE
			return 1; 
		}
		start_up_cnt++;
	}
	
	//////////////////////////////////////////////////
	// Подлючение HSE к PLL и запуск PLL
	//////////////////////////////////////////////////
	
	RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE_PREDIV_DIV1; // HSE / 1
	RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV; // HSE -> PLL
	RCC->CFGR |= RCC_CFGR_PLLMUL9; // HSE * 9
	start_up_cnt = 0; 
	RCC->CR |= RCC_CR_PLLON; // включаем PLL
	while(!(RCC->CR & RCC_CR_PLLRDY)){ // ждем пока выставится флаг ready для PLL
		if(start_up_cnt > 0x10000){ // если прошло много времени то отключаем настройки
			RCC->CR &= ~RCC_CR_PLLON;
			RCC->CR &= ~RCC_CR_HSEON;
			return 2;
		}
		start_up_cnt++;
	}
	
	//////////////////////////////////////////////////
	// Настройка делителей шин (AHB, APB1, APB2) и
	// настройка Flassh
	//////////////////////////////////////////////////
	
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // AHB = 72 МГц
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 = 36 МГц, т.к. это ее максималь фозмоная скорость
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // APB1 = 72 МГц
	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	FLASH->ACR |= FLASH_ACR_LATENCY_1; // т.к. 72Мгц  
	
	//////////////////////////////////////////////////
	// Использование PLL как SYSCLK 
	//////////////////////////////////////////////////
	
	RCC->CFGR |= RCC_CFGR_SW_PLL; // устанавливаем PLL как SYSCLK
	
	RCC->CR &= ~RCC_CR_HSION; // отключаем HSI
	
	return 0;
}

