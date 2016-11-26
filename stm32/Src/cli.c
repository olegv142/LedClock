#include "cli.h"
#include "uart.h"
#include "debug.h"
#include "main.h"
#include "leds.h"

#define STRZ_LEN(str) (sizeof(str)-1)

static const char s_cli_info[] = "LED clock terminal. Commands available:\n"
" t          get time\n"
" t hh mm ss set time\n"
" l          clear LEDs\n"
" l n r g b  setup n-th LED\n"
" ?          this help\n"
" @<string>  echo test\n"
;

#define CLI_OK "\n"
#define CLI_ERR "err\n"

static inline void cli_ok(void)
{
	uart_tx_string(CLI_OK, STRZ_LEN(CLI_OK));
}

static inline void cli_err(void)
{
	uart_tx_string(CLI_ERR, STRZ_LEN(CLI_ERR));
}

static inline void cli_resp(HAL_StatusTypeDef res)
{
	if (res == HAL_OK)
		cli_ok();
	else
		cli_err();
}

static void cli_get_time(void)
{
	RTC_TimeTypeDef time;
	HAL_StatusTypeDef res = HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	if (res != HAL_OK)
		cli_err();
	else
		uart_printf("%02u %02u %02u", time.Hours, time.Minutes, time.Seconds);
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

void cli_process(void)
{
	char cmd;
	if (!g_uart_rx_completed)
		return;
	BUG_ON(!g_uart_rx_len);
	cmd = g_uart_rx_buff[0];
	switch (cmd) {
	case 't':
		cli_time();
		break;
	case 'l':
		cli_led();
		break;
	case '?':
		uart_tx_string(s_cli_info, STRZ_LEN(s_cli_info));
		break;
	case '@':
		uart_tx_string(g_uart_rx_buff, g_uart_rx_len);
		break;
	default:
		uart_printf("invalid command '%c'", cmd);
	}
}
