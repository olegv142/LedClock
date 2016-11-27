#pragma once

#include "platform.h"

#define NLEDS 60

void leds_init(SPI_HandleTypeDef* h);
void leds_clear(void);
void leds_set(unsigned i, uint8_t r, uint8_t g, uint8_t b);
void leds_flush(void);
