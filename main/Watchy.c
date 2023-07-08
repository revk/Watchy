/* Watchy app */
/* Copyright ©2023 Adrian Kennard, Andrews & Arnold Ltd.See LICENCE file for details .GPL 3.0 */

static const char __attribute__((unused)) TAG[] = "Watchy";

#include "revk.h"
#include "esp_sleep.h"
#include "esp_task_wdt.h"
#include <driver/gpio.h>
#include <driver/uart.h>
#include <driver/rtc_io.h>
#include "gfx.h"
#include "ertc.h"

const char *gfx_qr (const char *value, gfx_pos_t posx, gfx_pos_t posy, uint8_t scale);  // QR
void face_init (void);          // Cold start up watch face
void face_show (struct tm *);   // Show current time

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
	u8(face,0)	\
	u8(i2cport,0)	\
	u8(rtcaddress,0x51)	\

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
night (struct tm *t)
{
   uint64_t mask = 0;
   for (int b = 0; b < 4; b++)
   {
      rtc_gpio_set_direction_in_sleep (port_mask (button[b]), RTC_GPIO_MODE_INPUT_ONLY);
      rtc_gpio_pullup_dis (port_mask (button[b]));
      rtc_gpio_pulldown_dis (port_mask (button[b]));
      mask |= (1ULL << port_mask (button[b]));
   }
   ESP_LOGI (TAG, "Night night %d", 60 - t->tm_sec);
   esp_sleep_enable_ext1_wakeup (mask, ESP_EXT1_WAKEUP_ANY_HIGH);
   esp_deep_sleep ((60 - t->tm_sec) * 1000000LL);
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
      uint8_t wakeup = esp_sleep_get_wakeup_cause ();
   if (!wakeup && esp_reset_reason () == ESP_RST_SW)
      wakeup = ESP_SLEEP_WAKEUP_ALL;    // It does not seem to say DEEPSLEEP, and does not always set a cause
   if (wakeup)
      ESP_LOGI (TAG, "Wake up %d", wakeup);

   // Buttons as soon as we can...
   for (int b = 0; b < 4; b++)
      if (button[b])
      {
         gpio_reset_pin (port_mask (button[b]));
         gpio_set_direction (port_mask (button[b]), GPIO_MODE_INPUT);
         gpio_pullup_dis (port_mask (button[b]));
         // TODO read state
      }

   if (mosi || dc || sck)
   {
      ESP_LOGI (TAG, "Start E-paper");
    const char *e = gfx_init (sck: port_mask (sck), cs: port_mask (ss), mosi: port_mask (mosi), dc: port_mask (dc), rst: port_mask (res), busy: port_mask (busy), flip: flip, width: 200, height: 200, partial: 1, mode2: 1, sleep: 1, norefresh:wakeup);
      if (e)
      {
         ESP_LOGE (TAG, "gfx %s", e);
         jo_t j = jo_object_alloc ();
         jo_string (j, "error", "Failed to start");
         jo_string (j, "description", e);
         revk_error ("gfx", &j);
      } else if (!wakeup)
         face_init ();
   }
   // Charging
   gpio_pullup_dis (rx);        // Used to detect the UART is down, and hence no VBUS and hence not charging.
   gpio_pulldown_en (rx);

   if (ertc_init ())
      ESP_LOGE (TAG, "RTC init fail");
   struct tm t;
   if (ertc_read (&t))
      ESP_LOGE (TAG, "RTC read fail");
   else if (wakeup && !gpio_get_level (rx))
   {                            // Fast display - TODO menu
      face_show (&t);
      night (&t);
   }

   revk_start ();

   ESP_LOGI (TAG, "Wait Time");
   while (time (0) < 120)
      sleep (1);                // TODO use RTC


   ESP_LOGI (TAG, "Main loop");
   while (1)
   {
      time_t now = time (0);
      struct tm t;
      localtime_r (&now, &t);
      if (now > 1000000000)
         ertc_write (&t);
      face_show (&t);
      if (!gpio_get_level (rx) || uptime () > 120)
         night (&t);
      else
         sleep (5);
   }
}
