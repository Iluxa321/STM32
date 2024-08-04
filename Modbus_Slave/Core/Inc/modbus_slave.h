/*
 * modbus_slave.h
 *
 *  Created on: Aug 14, 2022
 *      Author: Ilya
 */

#ifndef INC_MODBUS_SLAVE_H_
#define INC_MODBUS_SLAVE_H_

#include "stm32f3xx_hal.h"
#include <stdint-gcc.h>
#include <string.h>



#define SLAVE_ADDRESS	(0x01)
#define BUFFER_SIZE		(255)

#define MODBUS_UART				USART1
#define MODBUS_USART_IQR		USART1_IRQn
#define MODBUS_BAUD_RATE		9600

#define MODBUS_Tx_Enable()		MODBUS_UART->CR1 |= USART_CR1_TE
#define MODBUS_Tx_Disable()		MODBUS_UART->CR1 &= ~USART_CR1_TE
#define MODBUS_Rx_Enable()		MODBUS_UART->CR1 |= USART_CR1_RE
#define MODBUS_Rx_Disable()		MODBUS_UART->CR1 &= ~USART_CR1_RE


#define MODBUS_TIMER			TIM2
#define MODBUS_TIMER_ON()		(MODBUS_TIMER->CR1 |= TIM_CR1_CEN)
#define MODBUS_TIMER_OFF()		(MODBUS_TIMER->CR1 &= ~TIM_CR1_CEN)
#define MODBUS_TIMER_CLEAN()	(MODBUS_TIMER->CNT = 0)

#define MODBUS_RECIEVE_BUFFER					DMA1_Channel5
#define MODBUS_RECIEVE_IQR						DMA1_Channel5_IRQn
#define MODBUS_RECIEVE_OVERFLOW_HANDLER			DMA1_Channel5_IRQHandler
#define MODBUS_RECIEVE_OVERFLOW_FLAG			DMA_ISR_TCIF5
#define MODBUS_RECIEVE_OVERFLOW_CFLAG			DMA_IFCR_CGIF5
#define MODBIS_RECIEVE_OVERFLOW_FLAG_CLEAN()	(DMA1->IFCR |= MODBUS_RECIEVE_OVERFLOW_CFLAG)
#define MODBUS_RECIEVE_OVERFLOW()				(DMA1->ISR & MODBUS_RECIEVE_OVERFLOW_FLAG)


#define MODBUS_SEND_BUFFER						DMA1_Channel2
#define MODBUS_SEND_IQR							DMA1_Channel2_IRQn
#define MODBUS_SEND_HANDLER						DMA1_Channel2_IRQHandler
#define MODBUS_SEND_COMPITE_FLAG				DMA_ISR_TCIF2
#define MODBUS_SEND_COMPITE_CFLAG				DMA_IFCR_CGIF2
#define MODBUS_SEND_COMPITE_FLAG_CLEAN()		(DMA1->IFCR |= MODBUS_SEND_COMPITE_CFLAG)
#define MODBUS_SEND_COMPITE()					(DMA1->ISR & MODBUS_SEND_COMPITE_FLAG)


#define MODBUS_BUFFER_Disable(channel)			(channel->CCR &= ~DMA_CCR_EN)
#define MODBUS_BUFFER_Enable(channel)			(channel->CCR |= DMA_CCR_EN)
#define MODBUS_BUFFER_WRITE_CNT(channel, N)		(channel->CNDTR = N)
#define MODBUS_BUFFER_READ_CNT(channel)			(channel->CNDTR)

///////////////////////////////////////////////////////////
// 				Обработчики прерывания
//////////////////////////////////////////////////////////

#define Request_Handler		USART1_IRQHandler
#define Reply_Handler		MODBUS_SEND_HANDLER
#define Overflow_Handler	MODBUS_RECIEVE_OVERFLOW_HANDLER

////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// 		MODBUS FUNCTION
////////////////////////////////////////////////////

#define Read_Holding_Registers		(0x03)
#define Read_Input_Registers		(0x04)
#define Write_Single_Register		(0x06)
#define Write_Multiple_registers	(0x10)

//////////////////////////////////////////////////////

typedef uint8_t error;

typedef struct modbusHeader {
	uint8_t address;
	uint8_t function;
}modbusHeader;

typedef struct readHoldingRegister {
	uint16_t start_address;
	uint16_t quantity_register;

}readHoldingRegister;

typedef struct readInputRegister {
	uint16_t start_address;
	uint16_t quantity_register;
}readInputRegister;

typedef struct writeSingleRegister {
	uint16_t register_address;
	uint16_t register_value;
}writeSingleRegister;


typedef struct  writeMultipleRegisters  {
	uint16_t start_address;
	uint16_t quantity_register;
	uint8_t byte_count;
}writeMultipleRegisters;


typedef enum MODBUS_STATE{
	IDLE,
	CHECK_REUEST,
	REUEST_PROCESSING,
	REPLY
}MODBUS_STATE;

typedef enum MODBUS_EVENT{
	NONE,
	REQUEST_DONE,
//	CRC_OK,
//	CRC_FALDE,
//	PROCESSING_OK,
//	PROCESSING_FALDE,
	REPLY_DONE

}MODBUS_EVENT;




typedef struct modbus_t {
	MODBUS_STATE state;
	MODBUS_EVENT event;
	modbusHeader header;
	uint8_t buffer[BUFFER_SIZE];
	uint8_t data_len;
}modbus_t;

extern modbus_t modbus;

void modbus_tx_buffer_init(void);
void modbus_rx_buffer_init(void) ;
void modbus_uart_init(void);
void modbus_timer_init(void);
void modbus_crc_init(void);
uint16_t modbus_crc_calculation(uint8_t *data, uint8_t len);

void modbus_init(modbus_t *modbus);
void modbus_receive_start(modbus_t *modbus);
void modbus_receive_stop(void);
void modbus_transmite_start(modbus_t *modbus, uint8_t size);

//   		REQUEST FUNCTION
error modbus_read_holding_register_request(modbus_t *modbus, readHoldingRegister *read_holding_register, uint8_t registers_size);
error modbus_write_single_register_request(modbus_t *modbus, writeSingleRegister *write_single_register, uint8_t registers_size);
error modbus_write_multiple_registers_request(modbus_t *modbus, writeMultipleRegisters *write_multiple_registers, uint8_t registers_size);
error modbus_read_input_register_request(modbus_t *modbus, readInputRegister *read_input_register, uint8_t registers_size);

//			RESPONSE FUNCTION
uint8_t modbus_read_holding_register_response(modbus_t *modbus, readHoldingRegister *read_holding_register, uint8_t *registers);
uint8_t modbus_write_multiple_registers_response(modbus_t *modbus, writeMultipleRegisters *write_multiple_registers, uint8_t *registers);
uint8_t modbus_write_multiple_registers_response(modbus_t *modbus, writeMultipleRegisters *write_multiple_registers, uint8_t *registers);
uint8_t modbus_error_response(modbus_t *modbus, error error_code);

MODBUS_STATE modbus_idle_handler(modbus_t *modbus);
MODBUS_STATE modbus_check_reqest_handler(modbus_t *modbus);
MODBUS_STATE modbus_reqest_processing_handler(modbus_t *modbus, uint8_t *registers, uint8_t size);
MODBUS_STATE modbus_reply_handler(modbus_t *modbus);
void modbus_handler(modbus_t *modbus, uint8_t *registers, uint8_t size);


void Request_Handler(void);
void Reply_Handler(void);
void Overflow_Handler(void);


#endif /* INC_MODBUS_SLAVE_H_ */
