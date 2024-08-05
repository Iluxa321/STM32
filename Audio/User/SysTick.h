#ifndef _SYSTICK_
#define _SYSTICK_

#include "RTE_Components.h"
#include CMSIS_device_header



void SysTickInit(void);
void delay_ms(uint64_t ms);
void delay_us(uint64_t us);
uint64_t millis(void);
uint64_t micros(void);
void SysTick_Handler(void); // небоходимо определить в основной программе
extern volatile uint64_t SysTick_cnt; // переменную необходимо объявить если испольщовать функцию delay_ms(uint32_t ms)


#endif
