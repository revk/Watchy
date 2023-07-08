/* Watchy app */
/* Copyright ©2023 Adrian Kennard, Andrews & Arnold Ltd.See LICENCE file for details .GPL 3.0 */

static const char __attribute__((unused)) TAG[] = "Watchy";

#include "revk.h"
#include "esp_sleep.h"
#include "esp_task_wdt.h"
#include <driver/gpio.h>
#include <driver/uart.h>
#include "gfx.h"
#include "face.h"

// Settings (RevK library used by MQTT setting command)
#define settings                \
	ioa(button,4,"26 25 35 4")	\
	io(ss,5)	\
	io(dc,10)	\
	io(res,9)	\
	io(sck,18)	\
	io(sda,21)	\
	io(scl,22)	\
	io(mosi,23)	\
	io(rtcint,27)	\
	io(adc,34)	\
	io(vib,13)	\
	io(accint2,12)	\
	io(accint1,14)	\
	io(busy,19)	\
	u8(flip,0)	\

#define	tx	1
#define	rx	3
#define	port_mask(x)	((x)&0x7F)
#define u32(n,d)        uint32_t n;
#define u32l(n,d)        uint32_t n;
#define s8(n,d) int8_t n;
#define s8n(n,d) int8_t n[d];
#define u8(n,d) uint8_t n;
#define u8r(n,d) uint8_t n,ring##n;
#define u16(n,d) uint16_t n;
#define u16r(n,d) uint16_t n,ring##n;
#define s8r(n,d) int8_t n,ring##n;
#define s16r(n,d) int16_t n,ring##n;
#define u8l(n,d) uint8_t n;
#define b(n) uint8_t n;
#define s(n,d) char * n;
#define io(n,d)           uint8_t n;
#define ioa(n,a,d)      uint8_t n[a];
settings
#undef ioa
#undef io
#undef u32
#undef u32l
#undef s8
#undef s8n
#undef u8
#undef u8r
#undef u16
#undef u16r
#undef s8r
#undef s16r
#undef u8l
#undef b
#undef s
const char *
app_callback (int client, const char *prefix, const char *target, const char *suffix, jo_t j)
{
   if (client || !prefix || target || strcmp (prefix, prefixcommand))
      return NULL;              // Not for us or not a command from main MQTT
   return NULL;
}

void
app_main ()
{
   revk_boot (&app_callback);
#define io(n,d)           revk_register(#n,0,sizeof(n),&n,"- "#d,SETTING_SET|SETTING_BITFIELD|SETTING_FIX);
#define ioa(n,a,d)           revk_register(#n,a,sizeof(*n),&n,"- "#d,SETTING_SET|SETTING_BITFIELD|SETTING_FIX);
#define b(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
#define u32(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u32l(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_LIVE);
#define s8(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_SIGNED);
#define s8n(n,d) revk_register(#n,d,sizeof(*n),&n,NULL,SETTING_SIGNED);
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u8r(n,d) revk_register(#n,0,sizeof(n),&n,#d,0); revk_register("ring"#n,0,sizeof(ring##n),&ring##n,#d,0);
#define u16(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u16r(n,d) revk_register(#n,0,sizeof(n),&n,#d,0); revk_register("ring"#n,0,sizeof(ring##n),&ring##n,#d,0);
#define s8r(n,d) revk_register(#n,0,sizeof(n),&n,#d,0); revk_register("ring"#n,0,sizeof(ring##n),&ring##n,#d,SETTING_SIGNED);
#define s16r(n,d) revk_register(#n,0,sizeof(n),&n,#d,0); revk_register("ring"#n,0,sizeof(ring##n),&ring##n,#d,SETTING_SIGNED);
#define u8l(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_LIVE);
#define s(n,d) revk_register(#n,0,0,&n,#d,0);
   settings
#undef io
#undef ioa
#undef u32
#undef u32l
#undef s8
#undef s8n
#undef u8
#undef u8r
#undef u16
#undef u16r
#undef s8r
#undef s16r
#undef u8l
#undef b
#undef s
      revk_start ();
   if (mosi || dc || sck)
   {
      ESP_LOGI (TAG, "Start E-paper");
    const char *e = gfx_init (sck: port_mask (sck), cs: port_mask (ss), mosi: port_mask (mosi), dc: port_mask (dc), rst: port_mask (res), busy: port_mask (busy), flip: flip, width: 200, height: 200, partial: 1, mode2: 1, sleep:1);
      if (!e)
         face_init ();
      else
      {
         ESP_LOGE (TAG, "gfx %s", e);
         jo_t j = jo_object_alloc ();
         jo_string (j, "error", "Failed to start");
         jo_string (j, "description", e);
         revk_error ("gfx", &j);
      }
   }
   gfx_wait ();
   // Buttons
   for (int b = 0; b < 4; b++)
      if (button[b])
      {
         gpio_reset_pin (port_mask (button[b]));
         gpio_set_direction (port_mask (button[b]), GPIO_MODE_INPUT);
      }
   // Charging
   gpio_pullup_dis(rx);	// Used to detect the UART is down, and hence no VBUS and hence not charging.
   gpio_pulldown_en(rx);

   while (1)
   {
      jo_t j = jo_object_alloc ();
      jo_bool (j, "charging", gpio_get_level (rx));
      jo_bool (j, "btn1", port_mask (button[0]));
      jo_bool (j, "btn2", port_mask (button[1]));
      jo_bool (j, "btn3", port_mask (button[2]));
      jo_bool (j, "btn4", port_mask (button[3]));
      revk_info ("gpio", &j);
      time_t now = time (0);
      struct tm t;
      localtime_r (&now, &t);
      face_time (&t);
      sleep (60 - t.tm_sec);
   }

}
