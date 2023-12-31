// Menus

static const char __attribute__((unused)) TAG[] = "Menu";

#include "revk.h"
#include "watchy.h"
#include "gfx.h"
#include "icons.h"
#include "menu.h"

static const int margin = 24;   // Some formatting
static const int left = 24;
static const int line = 22;

RTC_NOINIT_ATTR uint8_t btnlast;

typedef void menu_fun_t (struct tm *, char);

const char *const list_face[] = {
#define face(n,d) #d,
#include "faces.m"
};

#define	menus	\
	m(face,Face select)	\
	m(wifi,WiFi settings)	\
	m(flip,Flip display)	\
	m(turn,Turn display)	\
	m(timezone,Set timezone)	\
	m(upgrade,S/W Upgrade)	\
	m(mqtt,MQTT connect)	\
	m(steps,Steps)	\
	m(info,Information)	\

#define	m(t,d) void menu_##t(struct tm *, char);
menus
#undef m
const char *const list_main[] = {
#define m(t,d)	#d,
   menus
#undef	m
};

enum
{
#define m(t,d)	mnum_##t,
   m (none,) menus
#undef	m
};

menu_fun_t *const fun_main[] = {
#define m(t,d)	&menu_##t,
   menus
#undef	m
};


void
gfx_menu1 (const char *title)
{
   gfx_pos (left - 3, gfx_y (), GFX_R | GFX_T);
   gfx_icon (right);
   gfx_pos (left, gfx_y (), GFX_L | GFX_T | GFX_H);
   gfx_text (-2, title);
}

void
gfx_menu (struct tm *t, const char *title)
{                               // Start menu
   uint16_t c = gfx_icon_size (right) / 2;
   gfx_lock ();
   gfx_clear (0);
   char temp[30];
   strftime (temp, sizeof (temp), "%F", t);
   gfx_pos (100, c, GFX_C | GFX_M);
   gfx_text (-2, title ? : temp);
   strftime (temp, sizeof (temp), "%H:%M %Z", t);
   gfx_pos (100, 199 - c, GFX_C | GFX_M);
   gfx_text (2, temp);
   gfx_pos (0, 0, GFX_L | GFX_T || GFX_H);
   gfx_icon (up);
   gfx_pos (199, 0, GFX_R | GFX_T | GFX_H);
   gfx_icon (right);
   gfx_pos (0, 199, GFX_L | GFX_B | GFX_H);
   gfx_icon (down);
   gfx_pos (c * 2, 199 - c, GFX_L | GFX_M);
   gfx_charging ();
   gfx_pos (199, 199, GFX_R | GFX_B | GFX_H);
   gfx_icon (left);
   gfx_pos (199 - c * 2, 199 - c, GFX_R | GFX_M);
   gfx_battery ();
   gfx_pos (100, 24, GFX_C | GFX_T | GFX_V);
   gfx_gap (5);
}

uint8_t
menu_list (struct tm *t, uint8_t pos, uint8_t len, const char *const m[], const char *title, char key)
{                               // Show a list, handle key U/D/L, return new position
   if (pos < 1)
      pos = 1;
   else if (pos > len)
      pos = len;
   if (key)
   {                            // State
      if (key == 'U' && pos > 1)
         pos--;
      else if (key == 'U' && pos == 1)
         pos = len;
      else if (key == 'D' && pos < len)
         pos++;
      else if (key == 'D' && pos == len)
         pos = 1;
      else if (key == 'L')
         pos = 0;
      return pos;
   }
   gfx_menu (t, title);
   const int lines = (200 - margin - margin) / line;
   int8_t base = pos - lines / 2;
   if (base + lines > len)
      base = len - lines;
   if (base < 1)
      base = 1;
   uint8_t y = margin;
   while (base <= len && y < 199 - margin)
   {
      if (base == pos)
      {
         gfx_pos (left - 3, y, GFX_R | GFX_T);
         gfx_icon (right);
      }
      gfx_pos (left, y + 1, GFX_L | GFX_T);
      gfx_text (-2, m[base - 1]);
      y += line;
      base++;
   }
   return pos;
}

void
menu_wifi (struct tm *t, char key)
{                               //  WiFi settings and AP mode
   if (key == 'L')
   {
      menu2 = 0;
      bits.holdoff = 0;
      return;
   }
   if (key == 'R' && menu2 == 0xFF)
      menu2 = 1;                // Selected
   bits.startup = 1;
   if (!bits.revkstarted)
      return;
   gfx_menu (t, "WiFi");
   char temp[30];
   gfx_text (-2, "Access point");
   snprintf (temp, sizeof (temp), "%s-%s", appname, revk_id);
   gfx_gap (10);
   gfx_text (strlen (temp) > 16 ? -1 : -2, "%s", temp);
   if (menu2 == 1)
   {                            // Selected
      bits.wifi = 1;
      bits.holdoff = 1;
      if (!bits.wifistarted)
         return;
      revk_command ("apconfig", NULL);
      menu2 = 2;
   } else if (menu2 == 2)
   {
      bits.wifi = 1;
      bits.holdoff = 1;
      bits.busy = 1;
      gfx_gap (10);
      gfx_text (2, "Connect to AP");
      gfx_gap (5);
      gfx_wifi ();
      gfx_mqtt ();
      wifi_mode_t mode = 0;
      esp_wifi_get_mode (&mode);
      if (mode != WIFI_MODE_AP && mode != WIFI_MODE_APSTA)
         menu2 = 255;           // AP mode closed
   } else if (menu2 == 0xFF)
   {
      gfx_gap (10);
      gfx_menu1 ("Start AP");
   }
   gfx_pos (100, 199 - margin, GFX_C | GFX_B);
   gfx_status ();
}

const char *const list_timezone[] = {
#define	tz(tag,...)	#tag,
#include "tzlist.m"
};

const char *const timezone[] = {
#define	tz(tag,...)	#__VA_ARGS__,
#include "tzlist.m"
};

void
menu_timezone (struct tm *t, char key)
{                               //  Timezone
   if (menu2 == 0xFF)
   {                            // Find timezone
      for (menu2 = 1; menu2 <= sizeof (timezone) / sizeof (*timezone); menu2++)
         if (!strcmp (rtctz, timezone[menu2 - 1]))
            break;
      if (menu2 > sizeof (timezone) / sizeof (*timezone))
         menu2 = 254;
   }
   if (menu2 == 254)
   {
      if (key == 'R')
         menu2 = 1;
      else if (key == 'L')
         menu2 = 0;
   } else if (key == 'R')
      menu3 = 1;                // Selected
   else
      menu2 = menu_list (t, menu2, sizeof (list_timezone) / sizeof (*list_timezone), list_timezone, "Timezone", key);
   if (!menu2)
      return;
   bits.startup = 1;
   if (!bits.revkstarted)
      return;
   if (menu2 == 254)
   {                            // Special case for not found
      gfx_menu (t, "Timezone");
      gfx_text (-2, "Has been set");
      gfx_text (-2, "via the WiFi");
      gfx_text (-2, "Settings");
      gfx_gap (5);
      gfx_text (strlen (rtctz) > 16 ? -1 : -2, "%s", rtctz);
      gfx_gap (10);
      gfx_menu1 ("Override");
   } else if (menu3)
   {                            // Selected
      menu1 = 0;
      jo_t j = jo_object_alloc ();
      jo_string (j, "tz", timezone[menu2 - 1]);
      revk_setting (j);
   }
}

void
menu_upgrade (struct tm *t, char key)
{                               //  Upgrade
   if (key == 'L' || key == 'R')
   {
      if (key == 'R')
         menu1 = 0;
      menu2 = 0;
      bits.wifi = 0;
      bits.holdoff = 0;
      return;
   }
   bits.wifi = 1;
   bits.holdoff = 1;
   if (!bits.revkstarted)
      return;
   gfx_menu (t, "Upgrade");
   extern const char *otahost;
   gfx_text (-2, "from...");
   gfx_text (strlen (otahost) > 16 ? -1 : -2, "%s", otahost);
   if (revk_link_down ())
   {
      gfx_gap (5);
      gfx_text (-2, "Waiting");
      gfx_gap (5);
      gfx_status ();
      return;
   }
   gfx_gap (5);
   int8_t percent = revk_ota_progress ();
   if (percent == -2)
      gfx_text (-2, "Up to date");
   else if (percent == 101)
      gfx_text (5, "Done");
   else if (percent >= 0 && percent <= 100)
      gfx_text (5, "%3d%%", percent);
   if (percent < 0 && uptime () > 120)
   {
      menu1 = 0;
      bits.wifi = 0;
      bits.holdoff = 0;
      return;
   }
   gfx_gap (5);
   gfx_status ();
   gfx_gap (5);
   if (percent >= 0)
   {
      gfx_pos (0, 199 - margin, GFX_B | GFX_L);
      gfx_fill (percent * 2, 10, 255);
      return;
   }
   const char *r = revk_command ("upgrade", NULL);
   if (r)
   {
      gfx_text (-2, "Trying");
      gfx_gap (5);
      gfx_text (-1, r);
      return;
   }
}

void
menu_face (struct tm *t, char key)
{                               //  Face select
   if (menu2 == 0xFF)
      menu2 = face + 1;
   if (key == 'R')
      menu3 = 1;                // Selected
   else
      menu2 = menu_list (t, menu2, sizeof (list_face) / sizeof (*list_face), list_face, "Face", key);
   if (!menu2)
      return;
   bits.startup = 1;
   if (!bits.revkstarted)
      return;
   if (menu3)
   {                            // Selected
      menu1 = 0;
      face = menu2 - 1;
      jo_t j = jo_object_alloc ();
      jo_int (j, "face", face);
      revk_setting (j);
   }
}

void
menu_flip (struct tm *t, char key)
{                               //  Flip display
   bits.startup = 1;
   if (!bits.revkstarted)
      return;
   flip ^= 3;
   jo_t j = jo_object_alloc ();
   jo_int (j, "flip", flip);
   revk_setting (j);
   jo_free (&j);
   gfx_flip (flip);
   menu1 = 0;
}

void
menu_turn (struct tm *t, char key)
{                               //  Turn display
   bits.startup = 1;
   if (!bits.revkstarted)
      return;
   flip ^= 5;
   jo_t j = jo_object_alloc ();
   jo_int (j, "flip", flip);
   revk_setting (j);
   jo_free (&j);
   gfx_flip (flip);
   menu1 = 0;
}

void
menu_steps (struct tm *t, char key)
{                               //  Steps
   if (key == 'L')
   {
      menu2 = 0;
      return;
   }
   if (key == 'R')
   {
      menu1 = 0;
      return;
   }
   bits.startup = 1;
   if (!bits.revkstarted)
      return;
   gfx_menu (t, "");
   t->tm_mday -= 7;
   const int top = 10;
   for (int i = 0; i < 7; i++)
   {
      t->tm_mday++;
      mktime (t);
      int32_t s = (i < 6 ? stepday[(t->tm_wday + 1) % 7] : steps) - stepday[t->tm_wday];
      char dow[4];
      strftime (dow, sizeof (dow), "%a", t);
      gfx_pos (left, top + i * 8 * 3, GFX_L | GFX_T | GFX_V);
      gfx_text (3, "%s", dow);
      if (s < 0)
         continue;
      gfx_pos (200 - left, top + i * 8 * 3, GFX_R | GFX_T | GFX_V);
      gfx_text (3, "%d", s);
   }
   gfx_pos (100, 0, GFX_C | GFX_T);
   gfx_text (1, "%d", steps);
}

void
menu_info (struct tm *t, char key)
{                               //  Info
   if (key == 'L')
   {
      menu2 = 0;
      return;
   }
   if (key == 'R')
   {
      menu1 = mnum_upgrade;
      return;
   }
   bits.startup = 1;
   if (!bits.revkstarted)
      return;
   gfx_menu (t, "Info");
   gfx_text (2, "MAC");
   gfx_gap (5);
   gfx_text (2, revk_id);
   gfx_gap (10);
   gfx_text (2, "WiFi");
   gfx_gap (5);
   extern const char *wifissid;
   gfx_text (strlen (wifissid) > 16 ? -1 : -2, "%s", wifissid);
   gfx_pos (100, 199 - margin, GFX_C | GFX_B);
   gfx_qr ("HTTPS://WATCHY.REVK.UK", 2);
   gfx_pos (50, 199 - margin, GFX_C | GFX_B);
   gfx_icon (ajk);
   gfx_pos (150, 199 - margin, GFX_C | GFX_B);
   gfx_icon (aa);
}

void
menu_mqtt (struct tm *t, char key)
{                               //  MQTT
   if (key == 'L')
   {
      menu2 = 0;
      return;
   }
   if (key == 'R' || uptime () > 110)
   {
      menu1 = 0;
      return;
   }
   bits.busy = 1;
   bits.wifi = 1;
   bits.startup = 1;
   if (!bits.revkstarted)
      return;
   gfx_menu (t, "MQTT");
   extern const char *wifissid;
   extern const char *mqtthost;
   if (!*wifissid)
   {
      gfx_text (-2, "WiFi not set");
      gfx_text (-2, "Use WiFi menu");
      return;
   }
   if (!*mqtthost)
   {
      gfx_text (-2, "MQTT not set");
      gfx_text (-2, "Use WiFi menu");
      return;
   }
   gfx_text (2, "Host");
   gfx_gap (5);
   gfx_text (strlen (mqtthost) > 16 ? -1 : -2, "%s", mqtthost);
   gfx_gap (10);
   gfx_text (2, "Name");
   gfx_gap (5);
   extern char *hostname;
   gfx_text (strlen (hostname) > 16 ? -1 : -2, "%s", hostname);
   gfx_gap (10);
   gfx_pos (50, gfx_y (), GFX_C | GFX_T);
   gfx_wifi ();
   gfx_pos (150, gfx_y (), GFX_C | GFX_T);
   gfx_mqtt ();
   gfx_pos (100, 199 - margin, GFX_C | GFX_B);
   gfx_status ();
}

void
menu_main (struct tm *t, char key)
{
   if (key == 'R')
      menu2 = 0xFF;             // Selected (0xFF tells it use starting point of choice)
   else
      menu1 = menu_list (t, menu1, sizeof (list_main) / sizeof (*list_main), list_main, NULL, key);
}

void
menu_show (struct tm *t, char key)
{
   ESP_LOGI (TAG, "Menu %d %d %d key %c flip %d started %d", menu1, menu2, menu3, key, flip, bits.revkstarted);
   if (key && !menu1)
   {
      menu1 = 1;                // Enter menu
      menu2 = 0;
      menu3 = 0;
      if (key == 'R')
      {                         // Quick 
         menu1 = mnum_steps;
         menu2 = 0xFF;
      }
      if (key == 'L')
      {                         // Quick 
         menu1 = mnum_info;
         menu2 = 0xFF;
      }
      key = 0;                  // used the key top enter menu
   }
   bits.busy = 0;               // Has to be set each time
   // Menu functions called with key set to update state and then called (after state change) with no key to display
   void process (char key)
   {
      if (!menu1)
         return;
      if (!menu2)
         menu_main (t, key);
      else if (menu1 && menu1 <= sizeof (list_main) / sizeof (*list_main))
         fun_main[menu1 - 1] (t, key);
   }
   if (key)
      process (key);
   process (0);
   gfx_unlock ();               // Always safe to extra unlock
}
