/* Watchy app */
/* Copyright ©2023 Adrian Kennard, Andrews & Arnold Ltd.See LICENCE file for details .GPL 3.0 */

static const char __attribute__((unused)) TAG[] = "Watchy";

#include "revk.h"
#include "watchy.h"
#include "esp_sleep.h"
#include "esp_sntp.h"
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
#ifndef	CONFIG_RTC_CLK_SRC_EXT_OSC
#warning You want CONFIG_RTC_CLK_SRC_EXT_OSC, I expect
#endif

bits_t bits = { 0 };

const uint8_t btn[] = { GPIOBTN2, GPIOBTN3, GPIOBTN1, GPIOBTN4 };

#define	BTNMASK	((1LL<<GPIOBTN1)|(1LL<<GPIOBTN2)|(1LL<<GPIOBTN3)|(1LL<<GPIOBTN4))

RTC_NOINIT_ATTR time_t moon_next;       // Next full moon
RTC_NOINIT_ATTR uint32_t stepbase;      // Current step count day base
RTC_NOINIT_ATTR uint32_t steps; // Current step count
RTC_NOINIT_ATTR uint8_t last_hour;      // Last hour number (to detect new hour)
RTC_NOINIT_ATTR uint8_t last_min;       // Last minute number (to detect new minute)
RTC_NOINIT_ATTR uint8_t last_btn;       // Last button state as turned in to keys
RTC_NOINIT_ATTR uint8_t battery;        // Current Battery level (percent)
RTC_NOINIT_ATTR uint8_t menu1;  // Current menu, levels
RTC_NOINIT_ATTR uint8_t menu2;
RTC_NOINIT_ATTR uint8_t menu3;
RTC_NOINIT_ATTR uint8_t moon_phase;
RTC_NOINIT_ATTR char rtctz[30]; // Current timezone string
RTC_NOINIT_ATTR char rtcdeadline[20];   // Deadline ISO time

// Settings (RevK library used by MQTT setting command)
#define settings                \
	u8lr(face,0)	\
	u8lr(flip,0)	\
	s8r(testday,0)	\
	u32al(stepday,7)\
	s(deadline,"0000-12-25")	\

#define	port_mask(x)	((x)&0x7F)
#define u32(n,d)        uint32_t n;
#define u32al(n,a)        uint32_t n[a];
#define u32l(n,d)        uint32_t n;
#define s8(n,d) int8_t n;
#define s8l(n,d) int8_t n;
#define s8n(n,d) int8_t n[d];
#define u8(n,d) uint8_t n;
#define u8r(n,d) RTC_NOINIT_ATTR uint8_t n,ring##n;
#define u16(n,d) uint16_t n;
#define s16(n,d) int16_t n;
#define s16lr(n,d) RTC_NOINIT_ATTR int16_t n;
#define u16r(n,d) uint16_t n,ring##n;
#define s8r(n,d) RTC_NOINIT_ATTR int8_t n,ring##n;
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
#undef u32al
#undef u32l
#undef s8
#undef s8l
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
   rtc_gpio_isolate (GPIOVIB);
   gfx_wait ();
   for (uint8_t b = 0; b < 4; b++)
   {
      rtc_gpio_set_direction_in_sleep (btn[b], RTC_GPIO_MODE_INPUT_ONLY);
      rtc_gpio_pullup_dis (btn[b]);
      rtc_gpio_pulldown_dis (btn[b]);
   }
   uint8_t secs = 60 - now % 60;
   if (last_btn)
   {                            // Wait release
      uint64_t mask = 0;
      for (uint8_t b = 0; b < 4; b++)
         if (last_btn & (1 << b))
            mask |= 1LL << btn[b];
      esp_sleep_enable_ext1_wakeup (mask, ESP_EXT1_WAKEUP_ALL_LOW);
      ESP_LOGD (TAG, "Wait key release %X, or %d seconds", last_btn, secs);
   } else
   {
      esp_sleep_enable_ext1_wakeup (BTNMASK, ESP_EXT1_WAKEUP_ANY_HIGH); // Wait press
      ESP_LOGD (TAG, "Wait key press, or %d seconds", secs);
   }
   esp_deep_sleep (1000000LL * secs ? : 600000LL);      // Next minute - or fast
}

void
read_steps (void)
{
   steps = acc_steps ();
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
timesync (struct timeval *tv)
{
   bits.timeunsync = 0;
   last_hour = tv->tv_sec / 3600 % 24;
   last_min = tv->tv_sec / 60 % 60;
   ertc_write (tv->tv_sec);
   ESP_LOGI (TAG, "Time sync @ %ld", uptime ());
}

static void
buzzer_task (void *pvParameters)
{
   ESP_LOGI (TAG, "Buzzer");
   if (bits.reset != ESP_RST_BROWNOUT || bits.charging)
   {
#if 0                           // Wants to be PWM or something, as just turning on seems to only work when charging?!
      gpio_set_level (GPIOVIB, 1);
      usleep (500000);
      gpio_set_level (GPIOVIB, 0);
#endif
   }
   bits.busy = 0;
   vTaskDelete (NULL);
}

static volatile char key1 = 0,
   key2 = 0;
static SemaphoreHandle_t key_mutex = NULL;

static void
key_check (uint64_t ext1)
{                               // Actually check for keys (TODO queue keys)
   uint8_t btns = 0;
   if (ext1)
   {
      for (uint8_t b = 0; b < 4; b++)
         if (ext1 & (1LL << btn[b]))
            btns |= (1 << b);
   } else
   {
      for (uint8_t b = 0; b < 4; b++)
         if (gpio_get_level (btn[b]))
            btns |= (1 << b);
   }
   last_btn &= btns;
   if (!key2)                   // Two key buffering
      for (uint8_t b = 0; b < 4; b++)
         if ((btns & (1 << b)) && !(last_btn & (1 << b)))
         {
            last_btn |= (1 << b);
            char key = "URDLLRDU"[(b ^ flip) & 7];      // Mapped for display flipping
            ESP_LOGI (TAG, "Key %d=%c (flip %X)", b, key, flip);
            xSemaphoreTake (key_mutex, portMAX_DELAY);
            if (key1)
               key2 = key;
            else
               key1 = key;
            xSemaphoreGive (key_mutex);
         }
}

static void
key_task (void *pvParameters)
{
   while (1)
   {
      key_check (0);
      usleep (1000);
   }
}

static char
next_key (void)
{
   if (!key1)
      return 0;
   char ret = 0;
   xSemaphoreTake (key_mutex, portMAX_DELAY);
   if (key1)
   {
      ret = key1;
      key1 = key2;
      key2 = 0;
   }
   xSemaphoreGive (key_mutex);
   return ret;
}

void
app_main ()
{
   bits.wakeup = esp_sleep_get_wakeup_cause ();
   bits.reset = esp_reset_reason ();
   {
      struct timeval tv;
      gettimeofday (&tv, NULL);

      ESP_LOGI (TAG, "Wake %d/%d @ %lld.%06ld", bits.reset, bits.wakeup, tv.tv_sec, tv.tv_usec);
   }
   if (!bits.wakeup)
      menu1 = menu2 = menu3 = 0;
   if (!bits.wakeup || bits.reset == ESP_RST_POWERON || bits.reset == ESP_RST_EXT || bits.reset == ESP_RST_BROWNOUT)
   {
      last_min = 255;
      last_hour = 255;
      moon_next = 0;
   }
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
   gpio_reset_pin (GPIOVIB);
   gpio_set_level (GPIOVIB, 0);
   gpio_set_direction (GPIOVIB, GPIO_MODE_OUTPUT);

   key_mutex = xSemaphoreCreateBinary ();
   xSemaphoreGive (key_mutex);
   key_check (last_btn ? 0 : esp_sleep_get_ext1_wakeup_status ());      // pick up as soon as possible, before task runs
   revk_task ("Key", key_task, NULL, 1);
   char key = next_key ();

   {                            // Time zone
      int l;
      for (l = 0; l < sizeof (rtctz) && rtctz[l]; l++);
      if (l < sizeof (rtctz))
      {
         setenv ("TZ", rtctz, 1);
         tzset ();
      } else
      {
         *rtctz = 0;
         ESP_LOGE (TAG, "TZ not set");
      }
      for (l = 0; l < sizeof (rtcdeadline) && rtcdeadline[l]; l++);
      if (l >= sizeof (rtcdeadline))
         strcpy (rtcdeadline, "0000-12-25");    // Default
   }

   void epaper_init (void)
   {
      if (gfx_ok ())
         return;
      ESP_LOGI (TAG, "Start E-paper flip=%d", flip);
    const char *e = gfx_init (sck: GPIOSCK, cs: GPIOSS, mosi: GPIOMOSI, dc: GPIODC, rst: GPIORES, busy: GPIOBUSY, flip: flip, width: 200, height: 200, partial: 1, mode2: 1, sleep: bits.wakeup ? 1 : 0, norefresh: 1, direct:1);
      if (e)
      {
         ESP_LOGE (TAG, "gfx %s", e);
         jo_t j = jo_object_alloc ();
         jo_string (j, "error", "Failed to start");
         jo_string (j, "description", e);
         revk_error ("gfx", &j);
      } else if (!bits.wakeup)
         face_init ();
   }

   if (i2c_init ())
      ESP_LOGE (TAG, "RTC init fail");
   if (bits.reset == ESP_RST_POWERON || bits.reset == ESP_RST_EXT || bits.reset == ESP_RST_BROWNOUT)
   {                            // Some h/w init
      ertc_init ();
      acc_init ();
      read_steps ();
   }
   time_t now = time (0) + 86400 * testday;
   if (now < 1000000000)
   {                            // We have no clock...
      now = ertc_read () + 86400 * testday;
      bits.wifi = 1;
      bits.timeunsync = 1;
   }
   if (now > 1000000000)
   {                            // Time updates
      uint8_t v = now / 3600 % 24;
      if (last_hour != v)
      {                         // New hour
         last_hour = v;
         bits.timeunsync = 1;
         bits.newhour = 1;
         bits.wifi = 1;
         read_battery ();
      }
      v = now / 60 % 60;
      if (last_min != v)
      {
         bits.newmin = 1;
         last_min = v;
         read_steps ();
      }
      if ((bits.newmin || key) && bits.wakeup)
      {                         // Draw ASAP
         epaper_init ();
         do
            face_show (now, key);
         while ((key = next_key ()));
      }
   } else
      bits.wifi = 1;            // Let's try and set clock

   if (bits.wakeup && !bits.wifi && !bits.holdoff && !key && !bits.startup && !bits.busy)
      night (now);

   // Full startup
   ESP_LOGI (TAG, "Revk boot wakeup=%d wifi=%d holdoff=%d key=%c", bits.wakeup, bits.wifi, bits.holdoff, key);
   bits.revkstarted = 1;
   revk_boot (&app_callback);
#define io(n,d)           revk_register(#n,0,sizeof(n),&n,"- "#d,SETTING_SET|SETTING_BITFIELD|SETTING_FIX);
#define ioa(n,a,d)           revk_register(#n,a,sizeof(*n),&n,"- "#d,SETTING_SET|SETTING_BITFIELD|SETTING_FIX);
#define b(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
#define u32(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u32al(n,a) revk_register(#n,a,sizeof(*n),&n,NULL,SETTING_LIVE);
#define u32l(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_LIVE);
#define s8(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_SIGNED);
#define s8l(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_SIGNED|SETTING_LIVE);
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
#undef u32al
#undef u32l
#undef s8
#undef s8l
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
      strncpy (rtcdeadline, deadline, sizeof (rtcdeadline));
   }

   if (!bits.wakeup || bits.newhour)
   {
      bits.busy = 1;
      revk_task ("Buzzer", buzzer_task, NULL, 2);
   }

   epaper_init ();

   if (bits.wifi)
   {
      bits.wifistarted = 1;
      revk_start ();            // Start WiFi

      sntp_set_time_sync_notification_cb (&timesync);
   }

   if (key || !bits.wakeup)
   {                            // Delayed
      now = ertc_read () + 86400 * testday;
      do
         face_show (now, key);
      while ((key = next_key ()));
   }

   if (bits.startup)
   {
      do
         face_show (now, key);
      while ((key = next_key ()));
   }

   if (!bits.busy && !bits.holdoff && !bits.timeunsync)
   {
      revk_pre_shutdown ();
      night (now);
   }

   time_t last = now;
   while (1)
   {
      now = time (0) + 86400 * testday;
      key = next_key ();
      if (now != last)
      {
         read_steps ();
         read_battery ();
      }
      if (key || now != last)
      {
         do
            face_show (now, key);
         while ((key = next_key ()));
      }
      last = now;
      if (bits.wifi && !bits.wifistarted)
      {                         // Start WiFi
         bits.wifistarted = 1;
         revk_start ();
         continue;
      }
      if (!bits.busy && !revk_shutting_down (NULL) && !(bits.timeunsync && uptime () < 20) && !(bits.holdoff && uptime () < 120))
      {
         revk_pre_shutdown ();
         night (now);           // Stay up in charging for 1 minute at least
      }
      usleep (100000);
   }
}
