/* Watchy app */
/* Copyright �2023 Adrian Kennard, Andrews & Arnold Ltd.See LICENCE file for details .GPL 3.0 */

static const char __attribute__((unused)) TAG[] = "Watchy";

#include "revk.h"
#include "watchy.h"
#include "esp_sleep.h"
#include "esp_task_wdt.h"
#include <driver/gpio.h>
#include <driver/uart.h>
#include <driver/rtc_io.h>
#include <driver/i2c.h>
#include "esp_adc/adc_oneshot.h"
#include "gfx.h"
#include "ertc.h"
#include "accelerometer.h"
#include "menu.h"

#ifdef	CONFIG_SECURE_SIGNED_ON_BOOT_NO_SECURE_BOOT
#warning Lower battery life if CONFIG_SECURE_SIGNED_ON_BOOT_NO_SECURE_BOOT
#endif

bits_t bits = { 0 };

const uint8_t btn[] = { GPIOBTN2, GPIOBTN3, GPIOBTN1, GPIOBTN4 };

#define	BTNMASK	((1LL<<GPIOBTN1)|(1LL<<GPIOBTN2)|(1LL<<GPIOBTN3)|(1LL<<GPIOBTN4))

RTC_NOINIT_ATTR int16_t last_adjust;
RTC_NOINIT_ATTR uint8_t last_hour;
RTC_NOINIT_ATTR uint8_t last_min;
RTC_NOINIT_ATTR uint8_t last_btn;
RTC_NOINIT_ATTR uint8_t epaper_refresh;
RTC_NOINIT_ATTR uint8_t battery;
RTC_NOINIT_ATTR uint8_t menu1;
RTC_NOINIT_ATTR uint8_t menu2;
RTC_NOINIT_ATTR uint8_t menu3;
RTC_NOINIT_ATTR char rtctz[30];

// Settings (RevK library used by MQTT setting command)
#define settings                \
	u8lr(face,0)	\
	u8lr(flip,5)	\
	s16lr(adjust,0)	\

#define	port_mask(x)	((x)&0x7F)
#define u32(n,d)        uint32_t n;
#define u32l(n,d)        uint32_t n;
#define s8(n,d) int8_t n;
#define s8n(n,d) int8_t n[d];
#define u8(n,d) uint8_t n;
#define u8r(n,d) RTC_NOINIT_ATTR uint8_t n,ring##n;
#define u16(n,d) uint16_t n;
#define s16(n,d) int16_t n;
#define s16lr(n,d) RTC_NOINIT_ATTR int16_t n;
#define u16r(n,d) uint16_t n,ring##n;
#define s8r(n,d) int8_t n,ring##n;
#define s16r(n,d) int16_t n,ring##n;
#define u8l(n,d) uint8_t n;
#define u8lr(n,d) RTC_NOINIT_ATTR uint8_t n;
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
#undef s16lr
#undef u16r
#undef s8r
#undef s16r
#undef u8l
#undef u8lr
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
night (time_t now)
{
   gfx_sleep ();
   for (uint8_t b = 0; b < 4; b++)
   {
      rtc_gpio_set_direction_in_sleep (btn[b], RTC_GPIO_MODE_INPUT_ONLY);
      rtc_gpio_pullup_dis (btn[b]);
      rtc_gpio_pulldown_dis (btn[b]);
   }
   uint8_t secs = 60 - now % 60;
   if (epaper_refresh)
   {
      epaper_refresh--;
      secs = 0;
   }
   if (last_btn)
   {                            // Wait release
      uint64_t mask = 0;
      for (uint8_t b = 0; b < 4; b++)
         if (last_btn & (1 << b))
            mask |= 1LL << btn[b];
      esp_sleep_enable_ext1_wakeup (mask, ESP_EXT1_WAKEUP_ALL_LOW);
      ESP_LOGI (TAG, "Wait key release %X, or %d seconds", last_btn, secs);
   } else
   {
      esp_sleep_enable_ext1_wakeup (BTNMASK, ESP_EXT1_WAKEUP_ANY_HIGH); // Wait press
      ESP_LOGI (TAG, "Wait key press, or %d seconds", secs);
   }
   esp_deep_sleep (1000000LL * secs ? : 600000LL);      // Next minute - or fast
}

void
read_battery (void)
{                               // ADC
   bits.charging = gpio_get_level (GPIORX) ? 1 : 0;
   adc_oneshot_unit_handle_t adc1_handle;
   adc_oneshot_unit_init_cfg_t init_config1 = {
      .unit_id = ADC_UNIT_1,
   };
   adc_oneshot_new_unit (&init_config1, &adc1_handle);
   adc_oneshot_chan_cfg_t config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_DB_11,
   };
   adc_oneshot_config_channel (adc1_handle, ADCCHANNEL, &config);
   int value;
   adc_oneshot_read (adc1_handle, ADCCHANNEL, &value);
   adc_oneshot_del_unit (adc1_handle);
   value = (value - BATLOW) * 100 / (BATHIGH - BATLOW);
   if (value < 0)
      value = 0;
   if (value > 100)
      value = 100;
   battery = value;
}

esp_err_t
i2c_init (void)
{
   i2c_config_t config = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = GPIOSDA,
      .scl_io_num = GPIOSCL,
      .sda_pullup_en = true,
      .scl_pullup_en = true,
      .master.clk_speed = 100000,
   };
   esp_err_t e = i2c_driver_install (I2CPORT, I2C_MODE_MASTER, 0, 0, 0);
   if (!e)
      e = i2c_param_config (I2CPORT, &config);
   if (!e)
      e = i2c_set_timeout (I2CPORT, 80000 * 5);
   return e;
}

void
app_main ()
{
   uint8_t wakeup = esp_sleep_get_wakeup_cause ();
   uint8_t reset = esp_reset_reason ();
   ESP_LOGI (TAG, "Wake %d/%d", reset, wakeup);
   if (!wakeup)
      menu1 = menu2 = menu3 = 0;

   // Charging
   gpio_pullup_dis (GPIORX);    // Used to detect the UART is down, and hence no VBUS and hence not charging.
   gpio_pulldown_en (GPIORX);
   bits.charging = gpio_get_level (GPIORX) ? 1 : 0;
   {
      gpio_config_t config = {
         .pin_bit_mask = BTNMASK,
         .mode = GPIO_MODE_INPUT,
      };
      gpio_config (&config);
   }
   char btn_read (void)
   {
      uint8_t btns = 0;
      for (uint8_t b = 0; b < 4; b++)
         if (gpio_get_level (btn[b]))
            btns |= (1 << b);
      last_btn &= btns;
      for (uint8_t b = 0; b < 4; b++)
         if ((btns & (1 << b)) && !(last_btn & (1 << b)))
         {
            last_btn |= (1 << b);
            char key = "RLUDRULD"[(b ^ flip) & 7];      // Mapped for display flipping
            ESP_LOGI (TAG, "Key %d=%c (flip %X)", b, key, flip);
            epaper_refresh = 20; // handle keys pressing too fast for display updates
            return key;
         }
      return 0;
   }
   char key = btn_read ();
   {                            // Time zone
      int l;
      for (l = 0; l < sizeof (rtctz) && rtctz[l]; l++);
      if (l < sizeof (rtctz))
      {
         setenv ("TZ", rtctz, 1);
         tzset ();
      } else
         ESP_LOGE (TAG, "TZ not set");
   }

   void epaper_init (void)
   {
      if (gfx_ok ())
         return;
      ESP_LOGI (TAG, "Start E-paper flip=%d", flip);
    const char *e = gfx_init (sck: GPIOSCK, cs: GPIOSS, mosi: GPIOMOSI, dc: GPIODC, rst: GPIORES, busy: GPIOBUSY, flip: flip, width: 200, height: 200, partial: 1, mode2: 1, sleep: wakeup ? 1 : 0, norefresh: wakeup ? 1 : 0, direct:1);
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

   time_t now = 0;
   if (i2c_init ())
      ESP_LOGE (TAG, "RTC init fail");
   if (reset == ESP_RST_POWERON || reset == ESP_RST_EXT || reset == ESP_RST_BROWNOUT)
   {                            // Some h/w init
      ertc_init ();
      acc_init ();
   }
   if (!(now = ertc_read ()))
      ESP_LOGE (TAG, "RTC read fail");
   else
   {
      uint8_t v = now / 60 % 60;
      if (last_min != v)
      {                         // Update display
         bits.newmin = 1;
         if (!(v % 5))
            read_battery ();
         last_min = v;
         if (wakeup)
         {
            epaper_init ();
            face_show (now, key);
            key = 0;
         }
      } else if (epaper_refresh && !key)
      {                         // Forced refresh
         epaper_init ();
         face_show (now, key);
      }
      if (wakeup == ESP_SLEEP_WAKEUP_TIMER)
      {                         // Per minute wake up
         v = now / 3600 % 24;
         if (last_hour != v)
         {                      // Normal start and attempt local clock sync
            bits.newhour = 1;
            last_hour = v;
            last_adjust = 0;
            bits.wifi = 1;
         } else
         {
            if (adjust)
            {                   // Not totally clean, but avoids the sleep wake up early at end of minute doing an adjust as well
               int16_t a = ((int) adjust * ((int) last_min + 1) / 60);
               if (a != last_adjust)
               {
                  ertc_write (now + a - last_adjust);
                  last_adjust = a;
               }
            }
         }
      }
   }

   if (wakeup && !bits.wifi && !bits.holdoff && !key && !bits.startup)
      night (now);

   // Full startup
   ESP_LOGI (TAG, "Revk boot wakeup=%d wifi=%d holdoff=%d key=%c", wakeup, bits.wifi, bits.holdoff, key);
   bits.revkstarted = 1;
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
#define s16lr(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_SIGNED);
#define u16r(n,d) revk_register(#n,0,sizeof(n),&n,#d,0); revk_register("ring"#n,0,sizeof(ring##n),&ring##n,#d,0);
#define s8r(n,d) revk_register(#n,0,sizeof(n),&n,#d,0); revk_register("ring"#n,0,sizeof(ring##n),&ring##n,#d,SETTING_SIGNED);
#define s16r(n,d) revk_register(#n,0,sizeof(n),&n,#d,0); revk_register("ring"#n,0,sizeof(ring##n),&ring##n,#d,SETTING_SIGNED);
#define u8l(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_LIVE);
#define u8lr(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_LIVE);
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
#undef s16lr
#undef u16r
#undef s8r
#undef s16r
#undef u8l
#undef u8lr
#undef b
#undef s
   {                            // RTC cached
      extern char *tz;
      strncpy (rtctz, tz, sizeof (rtctz));
   }

   epaper_init ();
   time_t last = now;

   if (bits.wifi || bits.charging)
   {
      bits.wifistarted = 1;
      revk_start ();            // Start WiFi

      ESP_LOGI (TAG, "Wait Time");
      while ((now = time (0)) < 30)
         sleep (1);
      if (now > 1000000000)
      {
         last_hour = now / 3600 % 24;
         last_min = now / 60 % 60;
         last_adjust = adjust * last_min / 60;
         ertc_write (now);
      }
   }

   if (key || !wakeup)
   {                            // Delayed
      now = ertc_read ();
      face_show (now, key);
   }

   if (!bits.holdoff)
   {
      revk_pre_shutdown ();
      night (now);
   }

   while (1)
   {
      read_battery ();
      now = ertc_read ();
      key = btn_read ();
      if (key || now != last)
         face_show (now, key);
      last = now;
      if (bits.wifi && !bits.wifistarted)
      {                         // Start WiFi
         bits.wifistarted = 1;
         revk_start ();
      }
      if (!revk_shutting_down (NULL) && (!bits.holdoff || uptime () > 120))
      {
         revk_pre_shutdown ();
         night (59);            // Stay up in charging for 1 minute at least
      }
      usleep (10000);
   }
}
