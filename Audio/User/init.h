#ifndef _INIT_
#define _INIT_

#include "RTE_Components.h"
#include CMSIS_device_header
#include "SysTick.h"

#define SIGNAL_OFFSET	((uint16_t)0x7F9) // сдвигаем на 2 т.к. используем 10 бит формат АЦП

void Timer2_init(void);
void Adc_init(void);
void dac_init(void);



#endif
