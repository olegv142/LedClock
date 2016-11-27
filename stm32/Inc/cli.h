#pragma once

#include "platform.h"
#include "uart.h"
#include "utils.h"

void cli_process(void);

#define CLI_EOL "\r"
#define CLI_OK CLI_EOL
#define CLI_ERR "err" CLI_EOL

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

