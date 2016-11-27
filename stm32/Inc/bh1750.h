#pragma once

#include "stm32f1xx_hal.h"

void bh1750init(I2C_HandleTypeDef* port);
int bh1750read(void);
