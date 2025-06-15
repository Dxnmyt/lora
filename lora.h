#ifndef SERIAL_H
#define SERIAL_H

#include "stm32f1xx_hal.h" // 根据你的实际 STM32 系列修改
#include <stdint.h>
#include <stdbool.h>

extern volatile uint32_t rx_interrupt_count;

HAL_StatusTypeDef Lora_UART_Init(UART_HandleTypeDef *huart_handle);
void Lora_UART_SendString(const char* str);
void Lora_UART_SendBytes(const uint8_t* data, uint16_t len);
int16_t Lora_UART_ReadByte(void);
uint16_t Lora_UART_ReadLine(char* line_buffer, uint16_t buffer_capacity, uint32_t timeout_ms);
void Lora_UART_ClearRxBuffer(void); // 添加清空缓冲区函数声明
void setup_lora_module(void);

#endif // SERIAL_H


