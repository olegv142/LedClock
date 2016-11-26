#include "cli.h"
#include "uart.h"
#include "debug.h"

static const char s_cli_info[] = "LED clock terminal. Commands available:\n"
" ?          this help\n"
" @<string>  echo test\n"
;

void cli_process(void)
{
	char cmd;
	if (!g_uart_rx_completed)
		return;
	BUG_ON(!g_uart_rx_len);
	cmd = g_uart_rx_buff[0];
	switch (cmd) {
	case '?':
		uart_tx_string(s_cli_info, sizeof(s_cli_info)-1);
		break;
	case '@':
		uart_tx_string(g_uart_rx_buff, g_uart_rx_len);
		break;
	default:
		uart_printf("invalid command '%c'", cmd);
	}
}
