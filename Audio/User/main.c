#include "main.h"


#define DAC_LOAD()	DAC1->SWTRIGR |= DAC_SWTRIGR_SWTRIG1
#define TIMER2_ON()	TIM2->CR1 |= TIM_CR1_CEN

#define DMA2DAC_chanle DMA1_Channel2
#define DMA_Chanel1_config (CIRC_En | PER2MEM | MINC_En | PINC_Dis | MSIZE_16 | PSIZE_16 | TCIE_En | HTIE_En)
#define DMA2DAC_chanle_config (CIRC_En | MEM2PER | MINC_En | PINC_Dis | MSIZE_16 | PSIZE_16 | TCIE_Dis | HTIE_Dis)





volatile uint64_t SysTick_cnt = 0;

q15_t ring_buffer[64] = {0,};	// буффер для уприема данных от АЦП
q15_t *p_buffer = ring_buffer;	// указатель на данные которые передаются в ЦАП
q31_t data[32];

volatile uint8_t flag = 0;



const q31_t cf2[5] = {1073741824,   -1073741824,     296081103,    1073741824, -387147875};
q31_t filt_st2[4] = {0};
arm_biquad_casd_df1_inst_q31 S2;
//const q31_t cf[5] = {1073741824,   -1073741824,     296081103,    1073741824, -387147875}; 
//q31_t filt_st[4] = {0};
//arm_biquad_casd_df1_inst_q31 S1;
const q31_t cf3[5] = {1073741824,   -1073741824,     296081103,    1073741824, -387147875};
q31_t filt_st3[4] = {0};
arm_biquad_casd_df1_inst_q31 S3;

const q31_t cf[5] = {21564350,   43128699,   21564350,    1073741824,    -688645971}; //LPF
q31_t filt_st[4] = {0};
arm_biquad_casd_df1_inst_q31 S1;


uint8_t filt_on = 1;

static void adc_start(void){
	ADC1->CR |= ADC_CR_ADSTART;
}

static void adc_stop(void){
	ADC1->CR |= ADC_CR_ADSTP;
}

static void dac_update(uint16_t data){
	while(DAC1->SWTRIGR); // ждем пока данные загрузятся
	DAC1->DHR12R1 = data;
	DAC_LOAD();

}

void conv_int2float(int16_t *a, float32_t *b, uint8_t block_size){
	for(int i = 0; i < block_size; i++){
		b[i] = (float32_t)a[i];
	}
}

void conv_float2int(float32_t *a, uint16_t *b, uint8_t block_size){
	for(int i = 0; i < block_size; i++){
		b[i] = (uint16_t)a[i];
	}
}



float32_t iir_filt(const float32_t *c, float32_t *in){
	static float32_t d1 = 0; // внутрение состояния фильтра
	static float32_t d2 = 0;
	float32_t out = 0;
	float32_t b0 = c[0];
	float32_t b1 = c[1];
	float32_t b2 = c[2];
	float32_t a1 = c[3];
	float32_t a2 = c[4];
	
	
	out = b0  * (*in) + d1;
	d1 = b1 * (*in) - a1 * (out) + d2;
	d2 = b2 * (*in) - a2 * (out);
	
	return out;
}

void iir_filt_block(const float32_t *coef, float32_t *in, float32_t *out, uint8_t block_size){
	for(int i = 0; i < block_size; i++){
		out[i] = iir_filt(coef, &in[i]);
	}
}




int main(void){
	ClockInit();
	SystemCoreClockUpdate();
	SysTickInit();
	

	NVIC_EnableIRQ(DMA1_Channel1_IRQn);
	//NVIC_EnableIRQ(DMA1_Channel2_IRQn);
	
	Adc_init();
	DMA_DeInit(DMA1_Channel1);
	DMA_Init(DMA1_Channel1, (uint32_t)&ADC1->DR, (uint32_t)ring_buffer, 64, DMA_Chanel1_config);
	DMA_DeInit(DMA2DAC_chanle);
	DMA_Init(DMA2DAC_chanle, (uint32_t)&DAC1->DHR12L1, (uint32_t)ring_buffer, 64, DMA2DAC_chanle_config);
	
	dac_init();
	Timer2_init();
	
	DMA1->IFCR |= DMA_IFCR_CTCIF1;
	DMA1->IFCR |= DMA_IFCR_CHTIF1;
	DMA1->IFCR |= DMA_IFCR_CTCIF2;
	DMA1->IFCR |= DMA_IFCR_CHTIF2;
	DMA_En(DMA1_Channel1);
	//DMA_En(DMA2DAC_chanle);

	
	//arm_biquad_cascade_df1_init_q31(&S1, 1, cf, filt_st, 1);
	//arm_biquad_cascade_df1_init_q31(&S2, 1, cf2, filt_st2, 1);
	//arm_biquad_cascade_df1_init_q31(&S3, 1, cf3, filt_st3, 1);
	
	for(int i = 0; i < NUMFILT; i++){
		arm_biquad_cascade_df1_init_q31(&ES[i], NUMSTAGE, (const q31_t *)&equalizer_coef[210*i + 10*(10+gain[i])], (q31_t *)&e_state[i][0], POSTSHIFT);
	}
	
/*
	arm_biquad_cascade_df1_init_q31(&ES0, NUMSTAGE, (const q31_t *)&equalizer_coef[210*0 + 10*(10+gain[0]/2)], (q31_t *)e_state0, POSTSHIFT);
	arm_biquad_cascade_df1_init_q31(&ES1, NUMSTAGE, (const q31_t *)&equalizer_coef[210*1 + 10*(10+gain[1]/2)], (q31_t *)e_state1, POSTSHIFT);
	arm_biquad_cascade_df1_init_q31(&ES2, NUMSTAGE, (const q31_t *)&equalizer_coef[210*2 + 10*(10+gain[2]/2)], (q31_t *)e_state2, POSTSHIFT);
	arm_biquad_cascade_df1_init_q31(&ES3, NUMSTAGE, (const q31_t *)&equalizer_coef[210*3 + 10*(10+gain[3]/2)], (q31_t *)e_state3, POSTSHIFT);
	
	arm_biquad_cascade_df1_init_q31(&ES4, NUMSTAGE, (const q31_t *)&equalizer_coef[210*4 + 10*(10+gain[4]/2)], (q31_t *)e_state4, POSTSHIFT);
	arm_biquad_cascade_df1_init_q31(&ES5, NUMSTAGE, (const q31_t *)&equalizer_coef[210*5 + 10*(10+gain[5]/2)], (q31_t *)e_state5, POSTSHIFT);
	arm_biquad_cascade_df1_init_q31(&ES6, NUMSTAGE, (const q31_t *)&equalizer_coef[210*6 + 10*(10+gain[6]/2)], (q31_t *)e_state6, POSTSHIFT);
*/
	
	adc_start();
	TIMER2_ON();
	
	
	while(1){
		
	}
}


void ADC1_2_IRQHandler(void){
	if(ADC1->ISR & ADC_ISR_OVR){
		abort();
	}
}

volatile uint64_t cycles_count = 0;

void DMA1_Channel1_IRQHandler(void){
	/*CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // Разрешаем счетчик тактов
	DWT->CYCCNT = 0; // Обнуляем счетчи
	cycles_count = DWT->CYCCNT; // Читаем счетчик тактов
	*/
	
	for(int i = 0; i < NUMFILT; i++){
		ES[i].pCoeffs = (const q31_t *)&equalizer_coef[210*i + 10*(10+gain[i])];
	}

	
	if(DMA1->ISR & DMA_ISR_HTIF1){
		DMA1->IFCR |= DMA_IFCR_CHTIF1;
		// Работа фильтра
		q15_t *in =  (q15_t *)&ring_buffer[0];
		arm_scale_q15(in, Q15_ABSMAX, -2, in, BLOCK_SIZE);
		//arm_shift_q15(in, 2, in, BLOCK_SIZE);
		if(filt_on){
			arm_q15_to_q31(in, data, BLOCK_SIZE);		
			
			
			
			
			for(int i = 0; i < NUMFILT; i++){
				arm_biquad_cascade_df1_fast_q31(&ES[i], (const q31_t *)data, data, BLOCK_SIZE);
			}

			
			
			
			arm_scale_q31(data, Q31_ABSMAX, 2, data, BLOCK_SIZE);
			arm_offset_q31((const q31_t *)data, (q31_t)(0.9*2147483648), (q31_t *)data, BLOCK_SIZE);
			arm_q31_to_q15(data, in, BLOCK_SIZE);
			//arm_shift_q15(in, 1, in, BLOCK_SIZE);
			
			
		}
		else{
			arm_offset_q15((const q15_t *)in, 0x4000, (q15_t *)in, BLOCK_SIZE);
		}
		//---------------------
		//p_buffer = in;	// сохроныем новые данные которые необходимо отправить
	}
	
	else if(DMA1->ISR & DMA_ISR_TCIF1){
		DMA1->IFCR |= DMA_IFCR_CTCIF1;
		// Работа фильтра
		q15_t *in =  (q15_t *)&ring_buffer[32];
		arm_scale_q15(in, Q15_ABSMAX, -2, in, BLOCK_SIZE);
		if(filt_on){
		
			arm_q15_to_q31(in, data, BLOCK_SIZE);	
			
			for(int i = 0; i < NUMFILT; i++){
				arm_biquad_cascade_df1_fast_q31(&ES[i], (const q31_t *)data, data, BLOCK_SIZE);
			}
			
			arm_scale_q31(data, Q31_ABSMAX, 2, data, BLOCK_SIZE);
			arm_offset_q31((const q31_t *)data, (q31_t)(0.9*2147483648), (q31_t *)data, BLOCK_SIZE);
			arm_q31_to_q15(data, in, BLOCK_SIZE);
			//arm_shift_q15(in, 1, in, BLOCK_SIZE);
			

		}
		else{
			arm_offset_q15((const q15_t *)in, 0x4000, (q15_t *)in, BLOCK_SIZE);
		}

		//---------------------
		//p_buffer = in;	// сохроныем новые данные которые необходимо отправить
	}
	
	if(!(DMA2DAC_chanle->CCR & DMA_CCR_EN) || flag == 0){ // в эту часть кода должны попадать только один раз (при старте программы)
		if(flag == 1){
			abort();
		}
		
		//DMA_Dis(DMA2DAC_chanle);
		//DMA2DAC_chanle->CMAR = (uint32_t)ring_buffer;
		//DMA2DAC_chanle->CNDTR = 64;
		
		DMA1->IFCR |= DMA_IFCR_CTCIF2;
		DMA1->IFCR |= DMA_IFCR_CHTIF2;
		DMA_En(DMA2DAC_chanle);
		
		flag = 1;
		
	}
}
/*
void DMA1_Channel2_IRQHandler(void){
	if(DMA1->ISR & DMA_ISR_TCIF2){
		DMA1->IFCR |= DMA_IFCR_CTCIF2;
		
		DMA_Dis(DMA2DAC_chanle);
		
		DMA2DAC_chanle->CMAR = (uint32_t)p_buffer;
		DMA2DAC_chanle->CNDTR = 32;
		
		DMA_En(DMA2DAC_chanle);
	}
}
*/

void SysTick_Handler(void){
	SysTick_cnt++;
}


// Реализация float
			/*conv_int2float((int16_t *)in, data, BLOCK_SIZE);
			arm_biquad_cascade_df1_f32 (&S, (const float32_t *)data, data, BLOCK_SIZE);
			arm_offset_f32((const float32_t *)data, 4096/2, data, BLOCK_SIZE);
			conv_float2int(data, in, BLOCK_SIZE);*/