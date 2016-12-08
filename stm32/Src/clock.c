#include "main.h"
#include "clock.h"
#include "leds.h"
#include "bh1750.h"
#include "config.h"
#include "debug.h"

static RTC_TimeTypeDef s_last_time;
static unsigned s_amb_light_avg;

static inline unsigned amb_light(void)
{
	return s_amb_light_avg >> 8;
}

static inline void amb_light_up(int a)
{
	if (a < 0) {
		s_amb_light_avg = 0;
		return;
	}
	s_amb_light_avg += a;
	s_amb_light_avg -= amb_light();
}

unsigned clk_amb_light(void)
{
	return amb_light();
}

void clk_init(void)
{
	HAL_StatusTypeDef res = HAL_RTC_GetTime(&hrtc, &s_last_time, RTC_FORMAT_BIN);
	BUG_ON(res != HAL_OK);
}

static int mins_elapsed(RTC_TimeTypeDef const* fr, RTC_TimeTypeDef const* to)
{
	int frm = 60 * fr->Hours + fr->Minutes;
	int tom = 60 * to->Hours + to->Minutes;
	int elapsed = tom - frm;
	while (elapsed > 60 * 12) {
		elapsed -= 60 * 24;
	}
	while (elapsed <= -60 * 12) {
		elapsed += 60 * 24;
	}
	return elapsed;
}

static inline uint8_t adjust_color(uint8_t c, int al, unsigned ah)
{
	if (c) {
		int a = amb_light() + 1; // just to avoid div by zero
		int v = c * (a + al);
		if (v <= 0) {
			return 0;
		}
		v /= (a + ah);
		if (al >= 0) {
			v += 1; // so v > 0
		}
		return v;
	} else
		return 0;
}

static inline void update_bk(struct config const* cfg)
{
	// background color
	uint8_t bk_r = adjust_color(cfg->bk.r, cfg->bl, cfg->bh);
	uint8_t bk_g = adjust_color(cfg->bk.g, cfg->bl, cfg->bh);
	uint8_t bk_b = adjust_color(cfg->bk.b, cfg->bl, cfg->bh);
	// hour marks color
	uint8_t hm_r = adjust_color(cfg->hm.r, cfg->al, cfg->ah);
	uint8_t hm_g = adjust_color(cfg->hm.g, cfg->al, cfg->ah);
	uint8_t hm_b = adjust_color(cfg->hm.b, cfg->al, cfg->ah);

	int i, next_hm = 0;
	for (i = 0; i < 60; ++i)
	{
		if (i == next_hm) {
			leds_set(i, hm_r, hm_g, hm_b);
			next_hm += 5;
		} else {
			leds_set(i, bk_r, bk_g, bk_b);
		}
	}
}

static inline void set_hh(unsigned i, struct config const* cfg)
{
	leds_set(i,
		adjust_color(cfg->hh.r, cfg->al, cfg->ah),
		adjust_color(cfg->hh.g, cfg->al, cfg->ah),
		adjust_color(cfg->hh.b, cfg->al, cfg->ah)
	);
}

static inline void set_mh(unsigned i, struct config const* cfg)
{
	leds_set(i,
		adjust_color(cfg->mh.r, cfg->al, cfg->ah),
		adjust_color(cfg->mh.g, cfg->al, cfg->ah),
		adjust_color(cfg->mh.b, cfg->al, cfg->ah)
	);
}

static inline void set_sh(unsigned i, struct config const* cfg)
{
	leds_set(i,
		adjust_color(cfg->sh.r, cfg->al, cfg->ah),
		adjust_color(cfg->sh.g, cfg->al, cfg->ah),
		adjust_color(cfg->sh.b, cfg->al, cfg->ah)
	);
}

static inline int get_hh_pos(RTC_TimeTypeDef const* t)
{
	return (t->Hours % 12) * 5 + t->Minutes / 12;
}

#define SR_LIGHT 0x10000

static void clk_update(RTC_TimeTypeDef const* t)
{
	struct config const* cfg = cfg_get();
	RTC_TimeTypeDef srt = {.Hours = cfg->srt.h, .Minutes = cfg->srt.m};
	int srt_elapsed = mins_elapsed(&srt, t);
	int hh = get_hh_pos(t);

	update_bk(cfg);
	set_hh(hh, cfg);
	set_mh(t->Minutes, cfg);
	if (cfg->se) {
		set_sh(t->Seconds, cfg);
	}
	leds_flush();

	s_last_time = *t;

	if (0 <= srt_elapsed && srt_elapsed < cfg->sr) {
		// simulate sunrise by substituting high ambient light instead of real one
		amb_light_up(SR_LIGHT);
	} else {
		amb_light_up(bh1750read());
	}
}

void clk_process(void)
{
	RTC_TimeTypeDef time;
	HAL_StatusTypeDef res = HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	BUG_ON(res != HAL_OK);
	if (s_last_time.Seconds != time.Seconds) {
		clk_update(&time);
	}
}

