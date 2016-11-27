#pragma once

int flash_erase_page(unsigned addr);
int flash_write(unsigned addr, void const* data, unsigned sz);

