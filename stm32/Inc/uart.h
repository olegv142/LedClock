#pragma once

#include "stm32f1xx_hal.h"

#define UART_BUFF_SZ 0x100

extern char g_uart_rx_buff[UART_BUFF_SZ];
extern char g_uart_tx_buff[UART_BUFF_SZ+1];

extern unsigned g_uart_tx_len;
extern unsigned g_uart_rx_len;
extern unsigned g_uart_tx_busy;
extern unsigned g_uart_rx_completed;

void uart_start(UART_HandleTypeDef* h);
void uart_tx_flush(void);
void uart_tx_string(const char* str, unsigned len);

static inline void uart_rx_reset(void)
{
	g_uart_rx_len = 0;
	g_uart_rx_completed = 0;
}
