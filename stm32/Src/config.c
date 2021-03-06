#include "config.h"
#include "platform.h"
#include "flash.h"
#include "debug.h"
#include "main.h"
#include "utils.h"
#include "uart.h"
#include "cli.h"

#include <stddef.h>
#include <string.h>

BUILD_BUG_ON(offsetof(struct config, crc) % sizeof(uint32_t));
BUILD_BUG_ON(sizeof(struct config) > FLASH_PAGE_SIZE);

static struct config s_cfg_default = {
	.bk = {100, 50, 50},
	.hm = {50, 0, 0},
	.hh = {0, 0, 255},
	.mh = {0, 255, 0},
	.sh = {150, 150, 0},
	.se = 1,
	.al = 5,
	.ah = 1000,
	.bl = -5000,
	.bh = 50000,
};

//------------- Configuration storage ---------------------

#define PAGE_ADDR(n) (FLASH_BASE+n*FLASH_PAGE_SIZE)

static __root uint8_t const s_cfg_page_1[FLASH_PAGE_SIZE] @ PAGE_ADDR(1);
static __root uint8_t const s_cfg_page_2[FLASH_PAGE_SIZE] @ PAGE_ADDR(2);

static unsigned s_cfg_page_addr[2] = {PAGE_ADDR(1), PAGE_ADDR(2)};
static int      s_cfg_idx = -1;

static int config_crc(struct config const* cfg)
{
	return HAL_CRC_Calculate(&hcrc, (uint32_t*)cfg, offsetof(struct config, crc) / sizeof(uint32_t));
}

static int config_valid(struct config const* cfg)
{
	return config_crc(cfg) == cfg->crc;
}

static inline int get_valid_config_idx(void)
{
	struct config const* cfg[2] = {
		(struct config const*)s_cfg_page_addr[0],
		(struct config const*)s_cfg_page_addr[1]
	};
	int valid[2] = {
		config_valid(cfg[0]),
		config_valid(cfg[1])
	};
	if (!valid[0] && !valid[1])
		return -1;
	if (!valid[0])
		return 1;
	if (!valid[1])
		return 0;
	return cfg[0]->gen > cfg[1]->gen ? 0 : 1;
}

void cfg_init(void)
{
	s_cfg_idx = get_valid_config_idx();
}

struct config const* cfg_get(void)
{
	return s_cfg_idx < 0 ? &s_cfg_default : (struct config const*)s_cfg_page_addr[s_cfg_idx];		
}

static inline int cfg_next_idx(void)
{
	return (s_cfg_idx + 1) % 2;
}

static int cfg_put_(struct config* cfg)
{
	int i = cfg_next_idx();
	cfg->gen = cfg_get()->gen + 1;
	cfg->crc = config_crc(cfg);
	if (
		flash_erase_page(s_cfg_page_addr[i]) ||
		flash_write(s_cfg_page_addr[i], cfg, sizeof(*cfg))
	)
		return -1;

	if (!config_valid((struct config const*)s_cfg_page_addr[i]))
		return -1;
	
	s_cfg_idx = i;
	return 0;
}

int cfg_put(struct config const* cfg)
{
	struct config new_cfg = *cfg;
	return cfg_put_(&new_cfg);
}

//------------- Command line interface ---------------------

struct config_item_tag {
	const char* name;
	const char* info;
	unsigned    offset;
	void (*print)(struct config const*, struct config_item_tag const*);
	int  (*scan)(struct config*, struct config_item_tag const*, const char*);
};

#define CFG_ITEM(fld, info, print, scan) {STR(fld), info, offsetof(struct config, fld), print, scan}

static void cfg_rgb_print(struct config const* cfg, struct config_item_tag const* t)
{
	struct rgb const* ptr = (struct rgb const*)((char const*)cfg + t->offset);
	uart_printf_("%s=%u %u %u " CLI_EOL, t->name, ptr->r, ptr->g, ptr->b);
}

static void cfg_hm_print(struct config const* cfg, struct config_item_tag const* t)
{
	struct hm const* ptr = (struct hm const*)((char const*)cfg + t->offset);
	uart_printf_("%s=%u %u " CLI_EOL, t->name, ptr->h, ptr->m);
}

static void cfg_uint_print(struct config const* cfg, struct config_item_tag const* t)
{
	unsigned const* ptr = (unsigned const*)((char const*)cfg + t->offset);
	uart_printf_("%s=%u " CLI_EOL, t->name, *ptr);
}

static void cfg_int_print(struct config const* cfg, struct config_item_tag const* t)
{
	int const* ptr = (int const*)((char const*)cfg + t->offset);
	uart_printf_("%s=%d " CLI_EOL, t->name, *ptr);
}

static int cfg_rgb_scan(struct config* cfg, struct config_item_tag const* t, const char* str)
{
	unsigned r = 0, g = 0, b = 0;
	struct rgb* ptr = (struct rgb*)((char*)cfg + t->offset);
	if (3 != sscanf(str, "%u %u %u", &r, &g, &b))
		return -1;
	ptr->r = r;
	ptr->g = g;
	ptr->b = b;
	return 0;
}

static int cfg_hm_scan(struct config* cfg, struct config_item_tag const* t, const char* str)
{
	unsigned h = 0, m = 0;
	struct hm* ptr = (struct hm*)((char*)cfg + t->offset);
	if (2 != sscanf(str, "%u %u", &h, &m))
		return -1;
	ptr->h = h;
	ptr->m = m;
	return 0;
}

static int cfg_uint_scan(struct config* cfg, struct config_item_tag const* t, const char* str)
{
	unsigned v = 0;
	unsigned* ptr = (unsigned*)((char*)cfg + t->offset);
	if (1 != sscanf(str, "%u", &v))
		return -1;
	*ptr = v;
	return 0;
}

static int cfg_int_scan(struct config* cfg, struct config_item_tag const* t, const char* str)
{
	int v = 0;
	int* ptr = (int*)((char*)cfg + t->offset);
	if (1 != sscanf(str, "%d", &v))
		return -1;
	*ptr = v;
	return 0;
}

static struct config_item_tag s_cfg_tags[] = {
	CFG_ITEM(bk, "r g b  - set background color",   cfg_rgb_print, cfg_rgb_scan),
	CFG_ITEM(hm, "r g b  - set hour marks color",   cfg_rgb_print, cfg_rgb_scan),
	CFG_ITEM(hh, "r g b  - set hour hand color",    cfg_rgb_print, cfg_rgb_scan),
	CFG_ITEM(mh, "r g b  - set minutes hand color", cfg_rgb_print, cfg_rgb_scan),
	CFG_ITEM(sh, "r g b  - set seconds hand color", cfg_rgb_print, cfg_rgb_scan),
	CFG_ITEM(srt,"h m    - set sunrise time",       cfg_hm_print, cfg_hm_scan),
	CFG_ITEM(sr, "mins   - set sunrise duration in minutes (0 if disabled)", cfg_uint_print, cfg_uint_scan),
	CFG_ITEM(se, "0|1    - disable (0) or enable (1) seconds hand", cfg_uint_print, cfg_uint_scan),
	CFG_ITEM(al, "value  - set ambient light low  threshold", cfg_uint_print, cfg_uint_scan),
	CFG_ITEM(ah, "value  - set ambient light high threshold", cfg_uint_print, cfg_uint_scan),
	CFG_ITEM(bl, "value  - set background low  threshold", cfg_int_print, cfg_int_scan),
	CFG_ITEM(bh, "value  - set background high threshold", cfg_uint_print, cfg_uint_scan),
};

void cfg_cli_info(void)
{
	int i;
	for (i = 0; i < ARR_LEN(s_cfg_tags); ++i) {
		struct config_item_tag const* t = &s_cfg_tags[i];
		uart_printf_("%s=%s" CLI_EOL, t->name, t->info);
	}
	uart_flush();
}

void cfg_cli_show(void)
{
	int i;
	struct config const* cfg = cfg_get();
	for (i = 0; i < ARR_LEN(s_cfg_tags); ++i) {
		struct config_item_tag const* t = &s_cfg_tags[i];
		t->print(cfg, t);
	}
	uart_flush();
}

void cfg_cli_set(const char* name, const char* str)
{
    int i;
	struct config cfg = *cfg_get();
	for (i = 0; i < ARR_LEN(s_cfg_tags); ++i) {
		struct config_item_tag const* t = &s_cfg_tags[i];
		if (!strcmp(name, t->name)) {
			int res = t->scan(&cfg, t, str);
			if (res)
				uart_printf("invalid config parameter value for '%s'" CLI_EOL, name);
			else if (cfg_put_(&cfg))
				uart_printf("failed to save configuration" CLI_EOL);
			else
				cli_ok();
			return;
		}
	}
	uart_printf("invalid config parameter name '%s'" CLI_EOL, name);
}


