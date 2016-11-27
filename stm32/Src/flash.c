#include "flash.h"
#include "platform.h"
#include "debug.h"

int flash_erase_page(unsigned addr)
{
	FLASH_EraseInitTypeDef er = {
		.TypeErase = FLASH_TYPEERASE_PAGES,
		.PageAddress = addr,
		.NbPages = 1,
	};
	uint32_t PageError = 0;
	HAL_FLASH_Unlock();
	HAL_StatusTypeDef res = HAL_FLASHEx_Erase(&er, &PageError);
	HAL_FLASH_Lock();
	return res == HAL_OK ? 0 : -1;
}

int flash_write(unsigned addr, void const* data, unsigned sz)
{
	HAL_StatusTypeDef res = HAL_OK;
	BUG_ON(addr % 2);
	HAL_FLASH_Unlock();
	for (; addr % 4 && sz >= 2; sz -= 2, addr += 2) {
		res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, *(uint16_t const*)data);
		data = (uint16_t const*)data + 1;
		if (res != HAL_OK)
			goto done;
	}
	for (; sz >= 4; sz -= 4, addr += 4) {
		res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *(uint32_t const*)data);
		data = (uint32_t const*)data + 1;
		if (res != HAL_OK)
			goto done;
	}
	for (; sz >= 2; sz -= 2, addr += 2) {
		res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, *(uint16_t const*)data);
		data = (uint16_t const*)data + 1;
		if (res != HAL_OK)
			goto done;
	}
done:
	HAL_FLASH_Lock();
	return res == HAL_OK ? 0 : -1;
}
