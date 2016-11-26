#include "leds.h"
#include "debug.h"

#include <string.h>

#define LEDS_BUFF_SZ (3*8*NLEDS+1)

#define LED_BIT0 0xc0 // 11000000
#define LED_BIT1 0xf8 // 11111000

static SPI_HandleTypeDef* s_leds_spi;
static uint8_t s_leds_buff[LEDS_BUFF_SZ];

void leds_init(SPI_HandleTypeDef* h)
{
	s_leds_spi = h;
	leds_clear();
	// transmit terminating byte to set output to idle level
	HAL_SPI_Transmit(h, &s_leds_buff[LEDS_BUFF_SZ-1], 1, HAL_MAX_DELAY);
}

void leds_clear(void)
{
	memset(s_leds_buff, LED_BIT0, LEDS_BUFF_SZ-1);
	s_leds_buff[LEDS_BUFF_SZ-1] = 0;
}

void leds_put_byte(unsigned i, uint8_t v)
{
	uint8_t m;
	unsigned j;
	for (m = 0x80, j = i*8; m; m >>= 1, ++j)
		s_leds_buff[j] = v & m ? LED_BIT1 : LED_BIT0;
}

void leds_set(unsigned i, uint8_t r, uint8_t g, uint8_t b)
{
	unsigned off = 3 * i;
	BUG_ON(i >= NLEDS);
	leds_put_byte(off++, g);
	leds_put_byte(off++, r);
	leds_put_byte(off++, b);
}

void leds_flush(void)
{
	BUG_ON(!s_leds_spi);
	HAL_SPI_Transmit_DMA(s_leds_spi, s_leds_buff, sizeof(s_leds_buff));
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
}
