#pragma once

#include "platform.h"

void bh1750init(I2C_HandleTypeDef* port);
int bh1750read(void);
