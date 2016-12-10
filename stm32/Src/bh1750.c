#include "bh1750.h"
#include "debug.h"

#define I2C_ADDR (0x23 << 1)
#define CMD_POWER_ON 1
#define CMD_CONT_HR_MODE  0x10
#define CMD_CONT_HR2_MODE 0x11

#define TOUT 1000

static I2C_HandleTypeDef* s_bh1750_port;

void bh1750init(I2C_HandleTypeDef* port)
{
	uint8_t cmd_pw_on = CMD_POWER_ON;
	uint8_t cmd_mode = CMD_CONT_HR2_MODE;
	HAL_StatusTypeDef res;
	s_bh1750_port = port;
    HAL_GPIO_WritePin(XRST_GPIO_Port, XRST_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
	res = HAL_I2C_Master_Transmit(port, I2C_ADDR, &cmd_pw_on, 1, TOUT);
	BUG_ON(res != HAL_OK);
	HAL_Delay(10);
	res = HAL_I2C_Master_Transmit(port, I2C_ADDR, &cmd_mode, 1, TOUT);
	BUG_ON(res != HAL_OK);
	HAL_Delay(200);
}

int bh1750read(void)
{
	uint8_t val[2];
	HAL_StatusTypeDef res;
	BUG_ON(!s_bh1750_port);
	res = HAL_I2C_Master_Receive(s_bh1750_port, I2C_ADDR, val, 2, TOUT);
	if (res != HAL_OK)
		return -1;
	return ((unsigned)val[0] << 8) | val[1];
}
