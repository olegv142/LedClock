#pragma once

#include "platform.h"

#define UART_BUFF_SZ 0x1000

extern char g_uart_rx_buff[UART_BUFF_SZ+1];
extern char g_uart_tx_buff[UART_BUFF_SZ+1];

extern unsigned g_uart_tx_len;
extern unsigned g_uart_rx_len;
extern unsigned g_uart_tx_busy;
extern unsigned g_uart_rx_completed;

void uart_init(UART_HandleTypeDef* h);

void uart_tx_string(const char* str, unsigned len);
void uart_printf(const char* fmt, ...);
void uart_printf_(const char* fmt, ...);
int  uart_scanf(const char* fmt, ...);

void uart_tx_flush(void);

static inline void uart_rx_reset(void)
{
	g_uart_rx_len = 0;
	g_uart_rx_completed = 0;
}

static inline void uart_flush(void)
{
	uart_rx_reset();
	uart_tx_flush();
}
