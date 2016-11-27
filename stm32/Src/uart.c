#include "uart.h"
#include "debug.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define UART_EOL '\n'
#define IS_EOL(c) ((c) == '\n' || (c) == '\r')
#define IS_WS(c) (IS_EOL(c) || (c) == ' ' || (c) == '\t')

static UART_HandleTypeDef* s_uart;
static uint8_t s_uart_buff;

char g_uart_rx_buff[UART_BUFF_SZ+1];
char g_uart_tx_buff[UART_BUFF_SZ+1];

unsigned g_uart_rx_len;
unsigned g_uart_tx_len;
unsigned g_uart_tx_busy;
unsigned g_uart_rx_completed;

void uart_init(UART_HandleTypeDef* h)
{
	s_uart = h;
	HAL_UART_Receive_IT(h, &s_uart_buff, 1);
}

static void uart_rx_handler(void)
{
	if (g_uart_rx_completed)
		return;
	if (!g_uart_rx_len && IS_WS(s_uart_buff))
		return;
	if (IS_EOL(s_uart_buff)) {
		BUG_ON(g_uart_rx_len > UART_BUFF_SZ);
		g_uart_rx_buff[g_uart_rx_len] = 0;
		g_uart_rx_completed = 1;
		return;
	}
	if (g_uart_rx_len >= UART_BUFF_SZ)
		return;
	g_uart_rx_buff[g_uart_rx_len] = s_uart_buff;
	++g_uart_rx_len;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uart_rx_handler();
	HAL_UART_Receive_IT(s_uart, &s_uart_buff, 1);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	BUG_ON(!g_uart_tx_busy);
	g_uart_tx_busy = 0;
}

void uart_tx_flush(void)
{
	BUG_ON(!s_uart);
	if (g_uart_tx_len) {
		BUG_ON(g_uart_tx_len > UART_BUFF_SZ);
		g_uart_tx_buff[g_uart_tx_len] = UART_EOL;
		HAL_UART_Transmit_IT(s_uart, (uint8_t*)g_uart_tx_buff, g_uart_tx_len + 1);
		g_uart_tx_len = 0;
		g_uart_tx_busy = 1;
	}
}

void uart_tx_string(const char* str, unsigned len)
{
	BUG_ON(g_uart_tx_busy);
	BUG_ON(g_uart_tx_len);
	BUG_ON(len > UART_BUFF_SZ);
	memcpy(g_uart_tx_buff, str, len);
	g_uart_tx_len = len;
	uart_flush();
}

void uart_printf_(const char* fmt, ...)
{
	int len;
	int space = UART_BUFF_SZ - g_uart_tx_len;
	va_list args;
	va_start(args, fmt);
	BUG_ON(space < 0);
	BUG_ON(g_uart_tx_busy);
	len = vsnprintf(g_uart_tx_buff + g_uart_tx_len, space, fmt, args);
	BUG_ON(len >= space);
	g_uart_tx_len += len;
	va_end(args);
}

void uart_printf(const char* fmt, ...)
{
	int len;
	va_list args;
	va_start(args, fmt);
	BUG_ON(g_uart_tx_busy);
	BUG_ON(g_uart_tx_len);
	len = vsnprintf(g_uart_tx_buff, UART_BUFF_SZ, fmt, args);
	BUG_ON(len >= UART_BUFF_SZ);
	g_uart_tx_len = len;
	uart_flush();
	va_end(args);
}

int uart_scanf(const char* fmt, ...)
{
	int res;
	va_list args;
	va_start(args, fmt);
	BUG_ON(!g_uart_rx_completed);
	res = vsscanf(g_uart_rx_buff, fmt, args);
	va_end(args);
	return res;
}
