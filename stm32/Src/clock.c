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

static inline uint8_t adjust_color(uint8_t c, unsigned al, unsigned ah, int bright)
{
	if (c) {
		unsigned a = amb_light() + 1; // just to avoid div by zero
		unsigned v = c * (a + al), d = a + ah;
		if (bright) {
			v = v + ah - al; // so v >= d
		}
		return v / d;
	} else
		return 0;
}

static inline void set_led_color(unsigned i, struct rgb const* c, unsigned al, unsigned ah, int bright)
{
	leds_set(i,
		adjust_color(c->r, al, ah, bright),
		adjust_color(c->g, al, ah, bright),
		adjust_color(c->b, al, ah, bright)
	);
}

static inline void set_bkg(unsigned i, struct config const* cfg)
{
	set_led_color(i, &cfg->bk, cfg->al, cfg->ab, 0);
}

static inline void set_hm(unsigned i, struct config const* cfg)
{
	set_led_color(i, &cfg->hm, cfg->al, cfg->ah, 1);
}

static inline void update_bk(unsigned fr, unsigned to, struct config const* cfg)
{
	while (fr != to)
	{
		if (!(fr % 5)) {
			set_hm(fr, cfg);
		} else {
			set_bkg(fr, cfg);
		}
		if (++fr >= 60) {
			fr -= 60;
		}
	}
}

static inline void set_hh(unsigned i, struct config const* cfg)
{
	set_led_color(i, &cfg->hh, cfg->al, cfg->ah, 1);
}

static inline void set_mh(unsigned i, struct config const* cfg)
{
	set_led_color(i, &cfg->mh, cfg->al, cfg->ah, 1);
}

static inline void set_sh(unsigned i, struct config const* cfg)
{
	set_led_color(i, &cfg->sh, cfg->al, cfg->ah, 1);
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

	update_bk(s_last_time.Seconds, t->Seconds, cfg);
	update_bk(s_last_time.Minutes, t->Minutes, cfg);
	update_bk(get_hh_pos(&s_last_time), hh, cfg);

	if (cfg->se) {
		set_sh(t->Seconds, cfg);
	}
	set_mh(t->Minutes, cfg);
	set_hh(hh, cfg);

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

