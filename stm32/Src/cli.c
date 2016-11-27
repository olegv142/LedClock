#include "cli.h"
#include "uart.h"
#include "debug.h"
#include "main.h"
#include "leds.h"
#include "bh1750.h"
#include "config.h"

#include <string.h>

#define TEST

static const char s_cli_info[] =
"LED clock terminal. Commands available:" CLI_EOL
" t          get time"              CLI_EOL
" t hh mm ss set time"              CLI_EOL
" x          get lux-meter reading" CLI_EOL
#ifdef TEST
" l          clear LEDs"            CLI_EOL
" l n r g b  setup n-th LED"        CLI_EOL
" @<string>  echo test"             CLI_EOL
#endif
" c          print config"          CLI_EOL
" <xx>=<val> set config value"      CLI_EOL
" c?         config help"           CLI_EOL
" ?          this help"             CLI_EOL
;

static void cli_get_time(void)
{
	RTC_TimeTypeDef time;
	HAL_StatusTypeDef res = HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	if (res != HAL_OK)
		cli_err();
	else
		uart_printf("%02u %02u %02u" CLI_EOL, time.Hours, time.Minutes, time.Seconds);
}

static void cli_set_time(unsigned h, unsigned m, unsigned s)
{
	RTC_TimeTypeDef time = {h, m, s};
	HAL_StatusTypeDef res = HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
	cli_resp(res);
}

static void cli_time(void)
{
	unsigned h, m, s;
	if (3 == uart_scanf("t %u %u %u", &h, &m, &s))
		cli_set_time(h, m, s);
	else
		cli_get_time();
}

static void cli_lux(void)
{
	uart_printf("%d" CLI_EOL, bh1750read());
}

#ifdef TEST

static void cli_led(void)
{
	unsigned n, r, g, b;
	if (4 == uart_scanf("l %u %u %u %u", &n, &r, &g, &b))
		leds_set(n, r, g, b);
	else
		leds_clear();

	leds_flush();
	cli_ok();
}

#endif

void cli_process(void)
{
	char cmd, *ptr;
	if (!g_uart_rx_completed)
		return;
	BUG_ON(!g_uart_rx_len);
	cmd = g_uart_rx_buff[0];
	switch (cmd) {
	case 't':
		cli_time();
		break;
	case 'x':
		cli_lux();
		break;
#ifdef TEST
	case 'l':
		cli_led();
		break;
	case '@':
		uart_tx_string(g_uart_rx_buff, g_uart_rx_len);
		break;
#endif
	case 'c':
		if (g_uart_rx_buff[1] == '?')
			cfg_cli_info();
		else
			cfg_cli_show();
		break;
	case '?':
		uart_tx_string(s_cli_info, STRZ_LEN(s_cli_info));
		break;
	default:
		if ((ptr = strchr(g_uart_rx_buff, '='))) {
			*ptr = 0;
			cfg_cli_set(g_uart_rx_buff, ptr + 1);
		} else
			uart_printf("invalid command '%c'" CLI_EOL, cmd);
	}
}
