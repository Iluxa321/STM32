#include "init.h"


void Timer2_init(void){
	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
	//NVIC_EnableIRQ(TIM2_IRQn); // global Interrupt enable
	TIM2->CR1 |= TIM_CR1_ARPE; 
	TIM2->CR2 |= TIM_CR2_MMS_1; // udpate event передаем как внешний тригер для АЦП
	TIM2->DIER |= TIM_DIER_UDE;	// разрешаем запрос по DMA (chanale 2) при обновлении таймера
	
	// Настрока таймера
	// T_event = core_clock/((PSC+1)(ARR+1))
	TIM2->PSC = 18 - 1; // 4 МГц
	TIM2->ARR = 100 - 1; // 100 тактов (25 мкс)

	TIM2->EGR |= TIM_EGR_UG;
	//TIM2->DIER |= TIM_DIER_UIE; // включаем прерывание
	
}


void Adc_init(void){
	// ADC_IN1 - PA0
	
	RCC->AHBENR |= RCC_AHBENR_ADC1EN;
	RCC->CFGR2 = RCC_CFGR2_ADC1PRES_NO; // используем шину AHB
	
	ADC1_COMMON->CCR |= (ADC1_CCR_CKMODE_0 | ADC1_CCR_CKMODE_1); // HCLK/4 (Synchronous clock mode) 72/4 = 18 Мгц
	
	
	// Calibration
	ADC1->CR &= ~ADC_CR_ADEN; // ADEN = 0
	ADC1->CR &= ~(ADC_CR_ADVREGEN_0 | ADC_CR_ADVREGEN_1); // для включения регулятора напряжения adc сперва переводим в 00
	ADC1->CR |= ADC_CR_ADVREGEN_0; // теперь включаем
	delay_us(15); // делаем задержку (минимум 10мкс)
	ADC1->CR &= ~ADC_CR_ADCALDIF;  // calibration in Single-ended inputs Mode
	ADC1->CR |= ADC_CR_ADCAL; // ADCAL = 1 (запускаем калибровку)
	while(ADC1->CR & ADC_CR_ADCAL); // Wait until ADCAL=0.
	
	ADC1->CR &= ~(ADC_CR_ADVREGEN_0 | ADC_CR_ADVREGEN_1); // для включения регулятора напряжения adc сперва переводим в 00
	ADC1->CR |= ADC_CR_ADVREGEN_1; // теперь включаем
	delay_us(15); // делаем задержку (минимум 10мкс)
	
	// Настрока АЦП
	
	NVIC_EnableIRQ(ADC1_2_IRQn); // разрешаем глобально прерывание
	//ADC1->IER |= ADC_IER_EOCIE; // разрешаем прерывания по окончанию оцифровки (для регулярной группы)
	ADC1->IER |= ADC_IER_OVRIE;
	
	
	
	ADC1->CFGR |= ADC_CFGR_EXTEN_0; // АЦП работает от внешенего тригера по переднему фронту
	ADC1->CFGR &= ~ADC_CFGR_EXTSEL;
	ADC1->CFGR |= (ADC_CFGR_EXTSEL_3 | ADC_CFGR_EXTSEL_1 | ADC_CFGR_EXTSEL_0); // вкачестве тригера выбираем таймер 2(TIM2_TRGO)
	
	ADC1->CFGR |= (ADC_CFGR_DMACFG | ADC_CFGR_DMAEN); // DMA Circular Mode selected; DMA enable
	
	ADC1->CFGR &= ~ADC_CFGR_CONT; // single mode
	ADC1->CFGR |= ADC_CFGR_ALIGN;
	//ADC1->CFGR |= ADC_CFGR_RES_0; // 10 bit
	ADC1->SQR1 |= (0x01 << ADC_SQR1_SQ1_Pos); // оцифровываем первый канал
	ADC1->SQR1 |= (0x00 << ADC_SQR1_L_Pos); // оцифровываем один канал
	ADC1->SMPR1 |= (ADC_SMPR1_SMP1_0 | ADC_SMPR1_SMP1_1); // 7.5 тактов на выборку
	// Полное время оцефровки состалвяет: (7.5 + 12.5) / (18*10^6) = 1.1 мкс
	
	// т.к. данные приходят со смещением, то необходимо вычесть это смещение
	ADC1->OFR1 |= ADC_OFR1_OFFSET1_EN;
	ADC1->OFR1 |= ADC_OFR1_OFFSET1_CH_0 | (SIGNAL_OFFSET << ADC_OFR1_OFFSET1_Pos);
	
	ADC1->CR |= ADC_CR_ADEN;
	while(!(ADC1->ISR & ADC_ISR_ADRDY));
}



void dac_init(void){
	// DAC - PA4
	RCC->APB1ENR |= RCC_APB1ENR_DAC1EN; // разрешаем тактирование DAC
	
	
	//DAC1->CR |= DAC_CR_DMAEN1;
	//DAC1->CR |= DAC_CR_TEN1; // trigger enable
	//DAC1->CR |= DAC_CR_TSEL1_2; // включаем программный тригер
	DAC1->CR |= DAC_CR_BOFF1; // output buffer enable
	DAC1->CR |= DAC_CR_EN1;
	
	
}