/* Watchy app */
/* Copyright ©2023 Adrian Kennard, Andrews & Arnold Ltd.See LICENCE file for details .GPL 3.0 */

static const char __attribute__((unused)) TAG[] = "Watchy";

#include "revk.h"
#include "watchy.h"
#include "esp_sleep.h"
#include "esp_task_wdt.h"
#include <driver/gpio.h>
#include <driver/uart.h>
#include <driver/rtc_io.h>
#include "gfx.h"
#include "ertc.h"
#include "menu.h"

const char *gfx_qr (const char *value, gfx_pos_t posx, gfx_pos_t posy, uint8_t scale);  // QR
void face_init (void);          // Cold start up watch face
void face_show (uint8_t, struct tm *);  // Show current time

// Settings (RevK library used by MQTT setting command)
#define settings                \
	u8(face,0)	\
	s16(adjust,0)	\

#define	port_mask(x)	((x)&0x7F)
#define u32(n,d)        uint32_t n;
#define u32l(n,d)        uint32_t n;
#define s8(n,d) int8_t n;
#define s8n(n,d) int8_t n[d];
#define u8(n,d) uint8_t n;
#define u8r(n,d) uint8_t n,ring##n;
#define u16(n,d) uint16_t n;
#define s16(n,d) int16_t n;
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
#undef s16
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
   // TODO isolate other pins?
   uint64_t mask = 0;
   void btn (uint8_t gpio)
   {
      rtc_gpio_set_direction_in_sleep (gpio, RTC_GPIO_MODE_INPUT_ONLY);
      rtc_gpio_pullup_dis (gpio);
      rtc_gpio_pulldown_dis (gpio);
      mask |= (1ULL << gpio);
   }
   btn (GPIOBTN1);
   btn (GPIOBTN2);
   btn (GPIOBTN3);
   btn (GPIOBTN4);
   esp_sleep_enable_ext1_wakeup (mask, ESP_EXT1_WAKEUP_ANY_HIGH);
   ESP_LOGE (TAG, "Night night %d", 60 - t->tm_sec);
   esp_deep_sleep ((60 - t->tm_sec) * 1000000LL);       // Next minute
}

int
awake (void)
{                               // Reasons to be awake
   if (gpio_get_level (GPIORX))
      return 1;                 // Charging
   if (revk_shutting_down (NULL))
      return 2;                 // Deliberate shutdown sequence (usually means OTA in progress)
   return 0;
}

void
app_main ()
{
   uint8_t wakeup = esp_sleep_get_wakeup_cause ();
   ESP_LOGE (TAG, "Start up %d", wakeup);

   // Charging
   gpio_pullup_dis (GPIORX);    // Used to detect the UART is down, and hence no VBUS and hence not charging.
   gpio_pulldown_en (GPIORX);
   uint8_t btn (int gpio)
   {
      gpio_reset_pin (gpio);
      gpio_set_direction (gpio, GPIO_MODE_INPUT);
      gpio_pullup_dis (gpio);
      return gpio_get_level (gpio);
   }
   uint8_t buttons = 0;
   if (btn (GPIOBTN1))
      buttons |= 1;
   if (btn (GPIOBTN2))
      buttons |= 2;
   if (btn (GPIOBTN3))
      buttons |= 4;
   if (btn (GPIOBTN4))
      buttons |= 8;

   void epaper_init (void)
   {
      static uint8_t done = 0;
      if (done)
         return;
      done = 1;
      ESP_LOGI (TAG, "Start E-paper");
    const char *e = gfx_init (sck: GPIOSCK, cs: GPIOSS, mosi: GPIOMOSI, dc: GPIODC, rst: GPIORES, busy: GPIOBUSY, flip: FLIP, width: 200, height: 200, partial: 1, mode2: 1, sleep: 1, norefresh:wakeup ? 1 : 0);
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

   static RTC_NOINIT_ATTR uint8_t rtcmenu;
   static RTC_NOINIT_ATTR uint8_t rtcface;
   static RTC_NOINIT_ATTR int16_t rtcadjust;
   static RTC_NOINIT_ATTR uint8_t last_hour;
   static RTC_NOINIT_ATTR int16_t last_adjust;

   struct tm t;
   if (ertc_init ())
      ESP_LOGE (TAG, "RTC init fail");
   else if (ertc_read (&t))
      ESP_LOGE (TAG, "RTC read fail");
   else
   {
      if (t.tm_sec < 5)
      {
         epaper_init ();
         if (rtcmenu)
            rtcmenu = menu_show (rtcmenu, buttons);
         else
            face_show (rtcface, &t);
      }
      if (wakeup == ESP_SLEEP_WAKEUP_TIMER)
      {                         // Per minute wake up
         if (last_hour != t.tm_hour)
         {                      // Normal start and attempt local clock sync
            last_hour = t.tm_hour;
            last_adjust = 0;
            ESP_LOGE (TAG, "Hourly wake");
         } else
         {
		 ESP_LOGE(TAG,"rtcadjust=%d",rtcadjust);
            if (rtcadjust)
            {                   // Not totally clean, but avoids the sleep wake up early at end of minute doing an adjust as well
               int16_t a = ((int) rtcadjust * (t.tm_min + 1) / 60);
               if (a != last_adjust)
               {
                  ESP_LOGE (TAG, "Adjust %d", (a - last_adjust));
                  t.tm_sec += (a - last_adjust);
                  ertc_write (&t);
                  last_adjust = a;
               }
            }
            night (&t);         // Allow normal start on the hour
         }
      }
   }


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
#define s16(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_SIGNED);
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
#undef s16
#undef u16r
#undef s8r
#undef s16r
#undef u8l
#undef b
#undef s
      revk_start ();
   rtcadjust = adjust;
   rtcface = face;
   if (!wakeup)
      rtcmenu = 0;

   epaper_init ();

   ESP_LOGI (TAG, "Wait Time");
   while (time (0) < 30)
      sleep (1);

   {                            // Set time
      struct timeval tv;
      gettimeofday (&tv, NULL);
      if (tv.tv_sec > 1000000000)
      {
         usleep (1000000 - tv.tv_usec);
         gettimeofday (&tv, NULL);
         struct tm t;
         localtime_r (&tv.tv_sec, &t);
         last_hour = t.tm_hour;
         last_adjust = rtcadjust * t.tm_min / 60;
         ertc_write (&t);
      }
   }

   while (1)
   {
      time_t now = time (0);
      struct tm t;
      localtime_r (&now, &t);
      if (rtcmenu)
         rtcmenu = menu_show (rtcmenu, buttons);
      else
         face_show (rtcface, &t);
      if (!awake () || uptime () > 120)
         night (&t);            // Stay up in charging for 2 minutes at least
      else
         sleep (1);
   }
}
