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
	unsigned   sr;  // sunrise duration in minutes (0 if disabled)
	unsigned   se;  // seconds enable
	unsigned   al;  // ambient light low threshold
	unsigned   ah;  // ambient light high threshold
	int        bl;  // background low threshold
	unsigned   bh;  // background high threshold
	uint32_t   gen; // generation number
	uint32_t   crc; // checksum
};

void cfg_init(void);

struct config const* cfg_get(void);

int cfg_put(struct config const* cfg);

void cfg_cli_info(void);

void cfg_cli_show(void);

void cfg_cli_set(const char* name, const char* str);
