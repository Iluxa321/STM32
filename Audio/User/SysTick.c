#include "SysTick.h"

#define SysClock 72000000
#define MS (SysClock / (1000 - 1))



void SysTickInit(void){
	// SysTick опрделена в файле core_cm4.h
	SysTick->LOAD &= ~SysTick_LOAD_RELOAD_Msk; // сбрасываем регистр LOAD в 0
	SysTick->LOAD = 9; // ”станавливаем значение хран€щеес€ в калибровочном регистре, т.к. работаем с частотай HCLKmax = 72 ћ√ц
	SysTick->VAL &= ~SysTick_VAL_CURRENT_Msk; // очищаем VAL
	SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk; // Clock = AHB / 8
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk; // разрешаем прервывание и разрешаем счет
}

uint64_t millis(void){
	return SysTick_cnt / 1000;
}

uint64_t micros(void){
	return SysTick_cnt;
}

void delay_ms(uint64_t ms){
	uint64_t timer = SysTick_cnt;
	while(SysTick_cnt - timer < ms*1000);
}

void delay_us(uint64_t us){
	uint64_t timer = SysTick_cnt;
	while(SysTick_cnt - timer < us);
}


