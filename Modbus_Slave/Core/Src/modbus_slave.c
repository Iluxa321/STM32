/*
 * modbus_slave.c
 *
 *  Created on: Aug 14, 2022
 *      Author: Ilya
 */

#include "modbus_slave.h"



void modbus_tx_buffer_init(void) {
	RCC->AHBENR |= RCC_AHBENR_DMA1EN; // разрешаем DMA

	MODBUS_SEND_BUFFER->CPAR = (uint32_t) &MODBUS_UART->TDR;
//	DMA1_Channel2->CMAR = (uint32_t) rx_txbuffer;
//	DMA1_Channel2->CNDTR = 8;
	MODBUS_SEND_BUFFER->CCR &= ~DMA_CCR_PINC;
	MODBUS_SEND_BUFFER->CCR |= DMA_CCR_MINC;
	MODBUS_SEND_BUFFER->CCR |= DMA_CCR_DIR;

	DMA1->IFCR = DMA_IFCR_CGIF2;

	MODBUS_SEND_BUFFER->CCR |= DMA_CCR_TCIE;

	NVIC_EnableIRQ(MODBUS_SEND_IQR);

}


void modbus_rx_buffer_init(void) {
	RCC->AHBENR |= RCC_AHBENR_DMA1EN; // разрешаем DMA

	MODBUS_RECIEVE_BUFFER->CPAR = (uint32_t) &MODBUS_UART->RDR;
	//	DMA1_Channel5->CMAR = (uint32_t) rx_txbuffer;
	//	DMA1_Channel5->CNDTR = 8;
	MODBUS_RECIEVE_BUFFER->CCR &= ~DMA_CCR_PINC;
	MODBUS_RECIEVE_BUFFER->CCR |= DMA_CCR_MINC;
	MODBUS_RECIEVE_BUFFER->CCR &= ~DMA_CCR_DIR;

	DMA1->IFCR = DMA_IFCR_CGIF5;

//	TODO: Расомтреть возможность переполнения
	MODBUS_RECIEVE_BUFFER->CCR |= DMA_CCR_TCIE;
	NVIC_EnableIRQ(MODBUS_RECIEVE_IQR);
}


void modbus_uart_init(void) {
	// USART1 Enable
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	// Настройка длины слова
	MODBUS_UART->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0); //  1 Start bit, 8 data bits, n stop bits
	// Настройка количества стоповых бит
	MODBUS_UART->CR2 &= ~USART_CR2_STOP; // 1 stop bit

	MODBUS_UART->BRR = (72000000 / 2) / MODBUS_BAUD_RATE;
	MODBUS_UART->RQR |= USART_RQR_RXFRQ; // очистеим флаг RXNE

	MODBUS_UART->CR3 |= USART_CR3_DMAT;	// разрешение работы DMA от передатчика
	MODBUS_UART->CR3 |= USART_CR3_DMAR;	// разрешение работы DMA от приемника
//		USART1->CR1 |= USART_CR1_TE;
	//	USART1->CR1 |= USART_CR1_RE;

	MODBUS_UART->CR1 |= USART_CR1_RTOIE;
	MODBUS_UART->CR2 |= USART_CR2_RTOEN;

	MODBUS_UART->RTOR = 45; // 3.5 character (1 charecter = 1 start + 8 bit + 1 stop)

	NVIC_EnableIRQ(MODBUS_USART_IQR);

	MODBUS_UART->CR1 |= USART_CR1_UE;

}


void modbus_timer_init(void) {
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

//	Расчет тербуемой задержки между данными modbus
//	(1 ch + 1 ch) * 10 / BAUD_RATE = 2ms
//	ARR = (72000000*20) / ((PSC+1)*BAUD_RATE) - 1
	MODBUS_TIMER->PSC = 10 - 1;
//	MODBUS_TIMER->ARR = 10000*2 -1;
	MODBUS_TIMER->ARR = ((72000000*20) / ((MODBUS_TIMER->PSC+1)*MODBUS_BAUD_RATE) - 1);


	MODBUS_TIMER->DIER |= TIM_DIER_UDE;
	MODBUS_TIMER->EGR |= TIM_EGR_UG;

}

void modbus_crc_init(void) {
	RCC->AHBENR |= RCC_AHBENR_CRCEN;
	CRC->CR |= CRC_CR_POLYSIZE_0;	// 16 bit полином
	CRC->CR |= CRC_CR_REV_IN_0;
	CRC->CR |= CRC_CR_REV_OUT;
	CRC->POL = 0x8005;
}

uint16_t modbus_crc_calculation(uint8_t *data, uint8_t len) {
	CRC->CR |= CRC_CR_RESET;
	for(int i = 0; i < len; i++) {
		*((volatile uint8_t *)&CRC->DR) = data[i];
	}
	return (uint16_t) CRC->DR;
}

void modbus_receive_start(modbus_t *modbus) {
	MODBUS_BUFFER_Disable(MODBUS_RECIEVE_BUFFER);
	MODBUS_RECIEVE_BUFFER->CMAR = (uint32_t) modbus->buffer;
	MODBUS_RECIEVE_BUFFER->CNDTR = BUFFER_SIZE;
	MODBUS_BUFFER_Enable(MODBUS_RECIEVE_BUFFER);
	MODBUS_Rx_Enable();
}

void modbus_receive_stop(void) {
	MODBUS_Rx_Disable();
	MODBUS_BUFFER_Disable(MODBUS_RECIEVE_BUFFER);
}

void modbus_transmite_start(modbus_t *modbus, uint8_t size) {
	MODBUS_BUFFER_Disable(MODBUS_SEND_BUFFER);
	MODBUS_SEND_BUFFER->CMAR = (uint32_t) modbus->buffer;
	MODBUS_SEND_BUFFER->CNDTR = size;
	MODBUS_BUFFER_Enable(MODBUS_SEND_BUFFER);
	MODBUS_Tx_Enable();
	MODBUS_TIMER_CLEAN();
	MODBUS_TIMER_ON();

}


void modbus_transmite_stop(void) {
	MODBUS_TIMER_OFF();
	MODBUS_Tx_Disable();
	MODBUS_BUFFER_Disable(MODBUS_SEND_BUFFER);
	MODBUS_TIMER_CLEAN();
}

void modbus_init(modbus_t *modbus) {
	modbus->state = IDLE;
	modbus->event = NONE;
	modbus->data_len = 0;
	modbus->header.address = 0;
	modbus->header.function = 0;
	memset((void*) modbus->buffer, 0, BUFFER_SIZE);

}


uint8_t modbus_read_holding_register_response(modbus_t *modbus, readHoldingRegister *read_holding_register, uint8_t *registers) {

//	Нужно ли делать выравнивание адресса по словам
//	Или оставить по байтам

	uint8_t nbyte;

	modbus->buffer[0] = modbus->header.address;
	modbus->buffer[1] = modbus->header.function;

	uint16_t start_address = read_holding_register->start_address*2;
	uint8_t cnt_byte = read_holding_register->quantity_register * 2;

	modbus->buffer[2] = cnt_byte;
	memcpy((void *)&modbus->buffer[3], (void *)&registers[start_address], cnt_byte);
	nbyte = 3 + cnt_byte;
	uint16_t crc = modbus_crc_calculation(modbus->buffer, nbyte);
	modbus->buffer[nbyte+1] = crc >> 8;
	modbus->buffer[nbyte] = (uint8_t) crc;
	nbyte = nbyte + 2;
	return nbyte;
}

uint8_t modbus_read_input_register_response(modbus_t *modbus, readInputRegister *read_input_register, uint8_t *registers) {
	uint8_t nbyte;

	modbus->buffer[0] = modbus->header.address;
	modbus->buffer[1] = modbus->header.function;

	uint16_t start_address = read_input_register->start_address*2;
	uint8_t cnt_byte = read_input_register->quantity_register * 2;

	modbus->buffer[2] = cnt_byte;
	memcpy((void *)&modbus->buffer[3], (void *)&registers[start_address], cnt_byte);
	nbyte = 3 + cnt_byte;
	uint16_t crc = modbus_crc_calculation(modbus->buffer, nbyte);
	modbus->buffer[nbyte+1] = crc >> 8;
	modbus->buffer[nbyte] = (uint8_t) crc;
	nbyte = nbyte + 2;
	return nbyte;

}

uint8_t modbus_write_multiple_registers_response(modbus_t *modbus, writeMultipleRegisters *write_multiple_registers, uint8_t *registers) {
	uint8_t nbyte = 6;
	modbus->buffer[0] = modbus->header.address;
	modbus->buffer[1] = modbus->header.function;
	modbus->buffer[2] = write_multiple_registers->start_address >> 8;
	modbus->buffer[3] = (uint8_t)write_multiple_registers->start_address;
	modbus->buffer[4] = write_multiple_registers->quantity_register >> 8;
	modbus->buffer[5] = (uint8_t)write_multiple_registers->quantity_register;
	uint16_t crc = modbus_crc_calculation(modbus->buffer, nbyte);
	modbus->buffer[nbyte+1] = crc >> 8;
	modbus->buffer[nbyte] = (uint8_t) crc;
	nbyte = nbyte + 2;
	return nbyte;

}


error modbus_read_holding_register_request(modbus_t *modbus, readHoldingRegister *read_holding_register, uint8_t registers_size) {
	error error_code = 0;
	read_holding_register->start_address = (modbus->buffer[2] << 8) | modbus->buffer[3];
	read_holding_register->quantity_register = (modbus->buffer[4] << 8) | modbus->buffer[5];
	if(read_holding_register->quantity_register < 0x0001 || read_holding_register->quantity_register > 0x007D) {
		error_code = 0x03;
	}
	else if((read_holding_register->start_address * 2) >= registers_size ||
			(read_holding_register->start_address * 2 + (read_holding_register->quantity_register * 2)) > registers_size) {
		error_code = 0x02;
	}
	return error_code;
}

error modbus_read_input_register_request(modbus_t *modbus, readInputRegister *read_input_register, uint8_t registers_size) {
	error error_code = 0;
	read_input_register->start_address = (modbus->buffer[2] << 8) | modbus->buffer[3];
	read_input_register->quantity_register = (modbus->buffer[4] << 8) | modbus->buffer[5];
	if(read_input_register->quantity_register < 0x0001 || read_input_register->quantity_register > 0x007D) {
		error_code = 0x03;
	}
	else if((read_input_register->start_address * 2) >= registers_size ||
			(read_input_register->start_address * 2 + (read_input_register->quantity_register * 2)) > registers_size) {
		error_code = 0x02;
	}
	return error_code;
}

error modbus_write_single_register_request(modbus_t *modbus, writeSingleRegister *write_single_register, uint8_t registers_size) {
	error error_code = 0;
	write_single_register->register_address = (modbus->buffer[2] << 8) | modbus->buffer[3];
	write_single_register->register_value = (modbus->buffer[4] << 8) | modbus->buffer[5];
	if((write_single_register->register_address * 2) >= registers_size) {
		error_code = 0x02;
	}
	return error_code;
}


error modbus_write_multiple_registers_request(modbus_t *modbus, writeMultipleRegisters *write_multiple_registers, uint8_t registers_size) {
	error error_code = 0;
	write_multiple_registers->start_address = (modbus->buffer[2] << 8) | modbus->buffer[3];
	write_multiple_registers->quantity_register = (modbus->buffer[4] << 8) | modbus->buffer[5];
	write_multiple_registers->byte_count = modbus->buffer[6];
	if(write_multiple_registers->quantity_register < 0x0001 ||
			write_multiple_registers->quantity_register > 0x007D ||
			write_multiple_registers->byte_count != write_multiple_registers->quantity_register * 2) {
		error_code = 0x03;
	}
	else if((write_multiple_registers->start_address * 2) >= registers_size ||
			(write_multiple_registers->start_address * 2 + (write_multiple_registers->quantity_register * 2)) > registers_size) {
		error_code = 0x02;
	}
	return error_code;
}

uint8_t modbus_error_response(modbus_t *modbus, error error_code) {
	uint8_t nbyte = 3;
	modbus->buffer[0] = modbus->header.address;
	modbus->buffer[1] = modbus->header.function + 0x80;
	modbus->buffer[2] = error_code;
	uint16_t crc = modbus_crc_calculation(modbus->buffer, nbyte);
	modbus->buffer[nbyte+1] = crc >> 8;
	modbus->buffer[nbyte] = (uint8_t) crc;
	nbyte = nbyte + 2;
	return nbyte;
}


MODBUS_STATE modbus_idle_handler(modbus_t *modbus) {
	MODBUS_STATE state = IDLE;
	if(modbus->event == REQUEST_DONE) {
		modbus->event = NONE;
		state = CHECK_REUEST;
	}
	return state;
}

MODBUS_STATE modbus_check_reqest_handler(modbus_t *modbus) {
	MODBUS_STATE state = CHECK_REUEST;
	uint16_t crc = 0;
	modbus->header.address = modbus->buffer[0];
	modbus->header.function = modbus->buffer[1];
	crc = modbus->buffer[modbus->data_len-1] << 8;
	crc |= modbus->buffer[modbus->data_len-2];
	uint16_t calc_crc = modbus_crc_calculation(modbus->buffer, modbus->data_len - 2);
	if(crc == calc_crc) {
//		modbus->event = CRC_OK;
		if(modbus->header.address != SLAVE_ADDRESS) {
			// Если адресс не совпадает, то не обрабатывает полученные данные
			modbus_receive_start(modbus);
			state = IDLE;
		}
		else {
			state = REUEST_PROCESSING;
		}
	}
	else {
		// Если CRC не верный то оставляем без ответа мастера
		modbus_receive_start(modbus);
		state = IDLE;
	}
	return state;
}

MODBUS_STATE modbus_reqest_processing_handler(modbus_t *modbus, uint8_t *registers, uint8_t size) {
	MODBUS_STATE state = REUEST_PROCESSING;
	switch(modbus->header.function) {
		case Read_Holding_Registers: {
			readHoldingRegister read_holding_register;
			error error_code = modbus_read_holding_register_request(modbus, &read_holding_register, size);
			if(error_code != 0) {
				uint8_t nbyte = modbus_error_response(modbus, error_code);
				modbus_transmite_start(modbus, nbyte);
			}
			else {
				uint8_t nbyte = modbus_read_holding_register_response(modbus, &read_holding_register, registers);
				modbus_transmite_start(modbus, nbyte);
			}
			state = REPLY;
			break;
		}
		case Read_Input_Registers: {
			readInputRegister read_input_register;
			error error_code = modbus_read_input_register_request(modbus, &read_input_register, size);
			if(error_code != 0) {
				uint8_t nbyte = modbus_error_response(modbus, error_code);
				modbus_transmite_start(modbus, nbyte);
			}
			else {
				uint8_t nbyte = modbus_read_input_register_response(modbus, &read_input_register, registers);
				modbus_transmite_start(modbus, nbyte);
			}
			state = REPLY;
			break;
		}
		case Write_Single_Register: {
			writeSingleRegister write_single_register;
			error error_code = modbus_write_single_register_request(modbus, &write_single_register, size);
			if(error_code != 0x00) {
				uint8_t nbyte = modbus_error_response(modbus, error_code);
				modbus_transmite_start(modbus, nbyte);
			}
			else {
				registers[write_single_register.register_address * 2 + 1] = write_single_register.register_value >> 8;
				registers[write_single_register.register_address * 2] = (uint8_t) write_single_register.register_value;
				modbus_transmite_start(modbus, 8);
			}
			state = REPLY;
			break;
		}
		case Write_Multiple_registers: {
			writeMultipleRegisters write_multiple_registers;
			error error_code = modbus_write_multiple_registers_request(modbus, &write_multiple_registers, size);
			if(error_code != 0x00) {
				uint8_t nbyte = modbus_error_response(modbus, error_code);
				modbus_transmite_start(modbus, nbyte);
			}
			else {
//				TODO: Не правельный порядок байт
//				Проверить можно ли менять порядок байт при отправлении в ОВЕН
//				memcpy((void *)&registers[write_multiple_registers.start_address*2], (void *)&modbus->buffer[7], (write_multiple_registers.byte_count));
				uint8_t *pdata = &modbus->buffer[7];
				for(int i = 0; i < write_multiple_registers.quantity_register; i++) {
					registers[write_multiple_registers.start_address*2 + i*2] = pdata[i*2 + 1];
					registers[write_multiple_registers.start_address*2 + i*2 + 1] = pdata[i*2];
				}

				uint8_t nbyte = modbus_write_multiple_registers_response(modbus, &write_multiple_registers, registers);
				modbus_transmite_start(modbus, nbyte);
			}
			state = REPLY;
			break;
		}
		default: {
			uint8_t nbyte = modbus_error_response(modbus, 0x01);
			modbus_transmite_start(modbus, nbyte);
			state = REPLY;
			break;
		}
	}
	return state;
}



MODBUS_STATE modbus_reply_handler(modbus_t *modbus) {
	MODBUS_STATE state = REPLY;
	if(modbus->event == REPLY_DONE) {
		modbus->event = NONE;
		modbus_receive_start(modbus);
		state = IDLE;
	}
	return state;
}

void modbus_handler(modbus_t *modbus, uint8_t *registers, uint8_t size) {
	switch(modbus->state) {
		case IDLE: {
			modbus->state = modbus_idle_handler(modbus);
			break;
		}
		case CHECK_REUEST: {
			modbus->state = modbus_check_reqest_handler(modbus);
			break;
		}
		case REUEST_PROCESSING: {
			modbus->state = modbus_reqest_processing_handler(modbus, registers, size);
			break;
		}
		case REPLY: {
			modbus->state = modbus_reply_handler(modbus);

			break;
		}
	}

}




void Request_Handler(void) {
	if(MODBUS_UART->ISR & USART_ISR_RTOF) {
		MODBUS_UART->ICR |= USART_ICR_RTOCF;

		modbus_receive_stop();

		modbus.data_len = (uint8_t)(BUFFER_SIZE - MODBUS_RECIEVE_BUFFER->CNDTR);
		modbus.event = REQUEST_DONE;

	}
}
void Reply_Handler(void) {
	if(MODBUS_SEND_COMPITE()) {
		MODBUS_SEND_COMPITE_FLAG_CLEAN();
		modbus_transmite_stop();
		modbus.event = REPLY_DONE;
	}
}
void Overflow_Handler(void){

}


