#pragma once

void Error_Handler(void);

#define BUG_ON(cond) do { if (cond) Error_Handler(); } while(0)
