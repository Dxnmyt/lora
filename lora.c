#include "serial.h"
#include "string.h" // For strlen
#include <stdio.h>
#include "usart.h"

static UART_HandleTypeDef* p_huart_lora = NULL;
static uint8_t single_byte_rx_buffer[1];
extern UART_HandleTypeDef huart1;

#define LORA_UART_RX_BUFFER_SIZE 256
volatile uint32_t rx_interrupt_count = 0;
static uint8_t lora_uart_rx_buffer_internal[LORA_UART_RX_BUFFER_SIZE]; 
static volatile uint16_t lora_uart_rx_write_idx_internal = 0;          
static volatile uint16_t lora_uart_rx_read_idx_internal  = 0;         

#define LORA_UART_DEFAULT_TX_TIMEOUT 1000 // Renamed for clarity

HAL_StatusTypeDef Lora_UART_Init(UART_HandleTypeDef *huart_handle) {
    // ...
    p_huart_lora = huart_handle; // 现在 huart_handle 是 &huart1
    HAL_StatusTypeDef status = HAL_UART_Receive_IT(p_huart_lora, single_byte_rx_buffer, 1);
    if (status != HAL_OK) {
        printf("Lora_UART_Init: HAL_UART_Receive_IT for %s failed with status: %d\r\n",
               (huart_handle->Instance == USART1) ? "USART1" : "USART2", status);
    } else {
        printf("Lora_UART_Init: HAL_UART_Receive_IT for %s started successfully.\r\n",
               (huart_handle->Instance == USART1) ? "USART1" : "USART2");
    }
    return status;
}

void Lora_UART_SendString(const char* cmd)
{
	if(p_huart_lora != NULL && cmd != NULL)
	{
		char full_buf[128];
		sprintf(full_buf, "%s\r\n", cmd);
		
		uint16_t acture_len = strlen(full_buf);
		if(acture_len > 0 && acture_len < sizeof(full_buf))
		{
			HAL_UART_Transmit(p_huart_lora, (uint8_t *)full_buf, acture_len, 1000);
		}
	}
}

void Lora_UART_SendBytes(const uint8_t* data, uint16_t len)
{
	if(p_huart_lora != NULL && data != NULL && len > 0)
	{
		HAL_UART_Transmit(p_huart_lora, (uint8_t *)data, len, LORA_UART_DEFAULT_TX_TIMEOUT);
	}
}

int16_t Lora_UART_ReadByte(void)
{
	if(lora_uart_rx_write_idx_internal != lora_uart_rx_read_idx_internal)
	{
		uint8_t byte = lora_uart_rx_buffer_internal[lora_uart_rx_read_idx_internal];
		lora_uart_rx_read_idx_internal = (lora_uart_rx_read_idx_internal + 1) % LORA_UART_RX_BUFFER_SIZE;
		return byte;
	}
	return -1;
}

uint16_t Lora_UART_ReadLine(char* line_buffer, uint16_t buffer_capacity, uint32_t timeout_ms)
{
	if(line_buffer == NULL || buffer_capacity == 0)
	{
		return 0;
	}

	uint16_t current_len = 0;
	uint32_t start_tick = HAL_GetTick();

	while(current_len < (buffer_capacity - 1))
	{
		if((HAL_GetTick() - start_tick) >= timeout_ms)
		{
			break;
		}

		int16_t byte_read = Lora_UART_ReadByte();
		if(byte_read != -1)
		{
			line_buffer[current_len++] = (char)byte_read;
			if((char)byte_read == '\n')
			{
				break;
			}
		}
		else
		{
			HAL_Delay(1);
		}
	}
	line_buffer[current_len] = '\0';
	return current_len;
}

void Lora_UART_ClearRxBuffer(void)
{
    lora_uart_rx_read_idx_internal = lora_uart_rx_write_idx_internal;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	printf("RX_Byte: 0x%02X\r\n", single_byte_rx_buffer[0]);
	rx_interrupt_count++;
	if(p_huart_lora != NULL && huart->Instance == p_huart_lora->Instance)
	{
		uint16_t next_wri = (lora_uart_rx_write_idx_internal + 1) % LORA_UART_RX_BUFFER_SIZE;
		if(next_wri != lora_uart_rx_read_idx_internal)
		{
			lora_uart_rx_buffer_internal[lora_uart_rx_write_idx_internal] = single_byte_rx_buffer[0];
			lora_uart_rx_write_idx_internal = next_wri;
		}
		else
		{
			
		}
		HAL_UART_Receive_IT(p_huart_lora, single_byte_rx_buffer, 1);
	}
}

int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 100);
  return ch;
}

void setup_lora_module(void) {
//    char response_part1[128];

//    Lora_UART_ClearRxBuffer();
//    printf("MD0 is HIGH. Sending AT command...\r\n"); // 假设MD0已正确设置
//    Lora_UART_SendString("ATE0"); // 发送 "AT"，函数内部加 "\r\n"

//    uint16_t len1 = Lora_UART_ReadLine(response_part1, sizeof(response_part1), 1000);
//    printf("Response Part 1 (len=%u): [%s]\r\n", len1, response_part1);
//	
//	if(len1 > 0 && strstr(response_part1, "OK") != NULL)
//	{
//		if(strstr(response_part1, "ATE0"))
//		{
//			Lora_UART_ClearRxBuffer();
//			Lora_UART_SendString("ATE0");
//			uint16_t len1 = Lora_UART_ReadLine(response_part1, sizeof(response_part1), 1000);
//		}
//		if (len1 > 0 && strstr(response_part1, "OK") != NULL)
//		{
//			printf("succ\r\n");
//		}
//		else
//		{
//			printf("err\r\n");
//		}
//	}
//	else
//	{
//		printf("err");
//	}
//	/* 测试
//	Lora_UART_ClearRxBuffer();
//	Lora_UART_SendString("AT");
//    len1 = Lora_UART_ReadLine(response_part1, sizeof(response_part1), 1000);
//    printf("Response Part 1 (len=%u): [%s]\r\n", len1, response_part1);
//	if(len1 > 0 && strstr(response_part1, "OK") != NULL)
//	{
//		printf("succ\r\n");
//	}
//	else
//	{
//		printf("err\r\n");
//	}
//	*/
//	//地址
//	Lora_UART_ClearRxBuffer();
//	Lora_UART_SendString("AT+ADDR=00,02");
//    len1 = Lora_UART_ReadLine(response_part1, sizeof(response_part1), 1000);
//    printf("Response Part 1 (len=%u): [%s]\r\n", len1, response_part1);
//	//发射功率
//	Lora_UART_ClearRxBuffer();
//	Lora_UART_SendString("AT+TPOWER=3");
//    len1 = Lora_UART_ReadLine(response_part1, sizeof(response_part1), 1000);
//    printf("Response Part 1 (len=%u): [%s]\r\n", len1, response_part1);
//	//AT+CWMODE=0配置工作模式
//	Lora_UART_ClearRxBuffer();
//	Lora_UART_SendString("AT+CWMODE=0");
//    len1 = Lora_UART_ReadLine(response_part1, sizeof(response_part1), 1000);
//    printf("Response Part 1 (len=%u): [%s]\r\n", len1, response_part1);
//	//AT+TMODE=0发射模式透明传输
//	Lora_UART_ClearRxBuffer();
//	Lora_UART_SendString("AT+TMODE=0");
//    len1 = Lora_UART_ReadLine(response_part1, sizeof(response_part1), 1000);
//    printf("Response Part 1 (len=%u): [%s]\r\n", len1, response_part1);
//	//AT+WLRATE=24,2配置空中速率和信道配置
//	Lora_UART_ClearRxBuffer();
//	Lora_UART_SendString("AT+WLRATE=24,2");
//    len1 = Lora_UART_ReadLine(response_part1, sizeof(response_part1), 1000);
//    printf("Response Part 1 (len=%u): [%s]\r\n", len1, response_part1);
//	//AT+FLASH=1保存配置
//	Lora_UART_ClearRxBuffer();
//	Lora_UART_SendString("AT+FLASH=1");
//    len1 = Lora_UART_ReadLine(response_part1, sizeof(response_part1), 1000);
//    printf("Response Part 1 (len=%u): [%s]\r\n", len1, response_part1);

}





