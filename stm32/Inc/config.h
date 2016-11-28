#pragma once

#include <stdint.h>

struct rgb {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct hm {
	uint8_t h;
	uint8_t m;
};

struct config {
	struct rgb bk;  // background color
	struct rgb hm;  // hour marks color
	struct rgb hh;  // hour hand color
	struct rgb mh;  // minutes hand color
	struct rgb sh;  // seconds hand color
	struct hm  srt; // sunrise time
	uint16_t   sr;  // sunrise duration in minutes (0 if disabled)
	uint16_t   se;  // seconds enable
	uint16_t   xh;  // lux-meter high threshold
	uint16_t   xl;  // lux-meter low threshold
	uint32_t   gen; // generation number
	uint32_t   crc; // checksum
};

void cfg_init(void);

struct config const* cfg_get(void);

int cfg_put(struct config const* cfg);

void cfg_cli_info(void);

void cfg_cli_show(void);

void cfg_cli_set(const char* name, const char* str);
