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

typedef void menu_fun (struct tm *, char);
typedef struct
{
   menu_fun *fun;
   const char *name;
} menulist_t;

menulist_t list_face[] = {
#define face(n,d) {NULL,#d},
#include "faces.m"
};

void
gfx_gap (uint8_t g)
{
   gfx_pos (gfx_x (), gfx_y () + g, gfx_a ());
}

void
gfx_menu (struct tm *t, const char *title)
{                               // Start menu
   gfx_lock ();
   gfx_clear (0);
   char temp[30];
   strftime (temp, sizeof (temp), "%F", t);
   gfx_pos (100, 0, GFX_C | GFX_T);
   gfx_text (-2, title ? : temp);
   strftime (temp, sizeof (temp), "%H:%M %Z", t);
   gfx_pos (100, 199, GFX_C | GFX_B);
   gfx_text (2, temp);
   gfx_pos (0, 0, GFX_L | GFX_T || GFX_H);
   gfx_icon (up);
   gfx_pos (199, 0, GFX_R | GFX_T | GFX_H);
   gfx_icon (right);
   gfx_pos (0, 199, GFX_L | GFX_B | GFX_H);
   gfx_icon (down);
   gfx_iconq (charging, bits.charging);
   gfx_pos (199, 199, GFX_R | GFX_B | GFX_H);
   gfx_icon (left);
   gfx_pos (gfx_x (), gfx_y () - 1, gfx_a ());
   gfx_battery ();
   gfx_pos (100, 24, GFX_C | GFX_T | GFX_V);
   gfx_gap (5);
}

uint8_t
menu_list (struct tm *t, uint8_t pos, uint8_t len, menulist_t * m, const char *title, char key)
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
      gfx_text (-2, m[base - 1].name);
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
      gfx_iconq (wifi, !revk_link_down ());
      gfx_iconq (mqtt, lwmqtt_connected (revk_mqtt (0)));
      wifi_mode_t mode = 0;
      esp_wifi_get_mode (&mode);
      if (mode != WIFI_MODE_AP && mode != WIFI_MODE_APSTA)
         menu2 = 255;           // AP mode closed
   } else if (menu2 == 0xFF)
   {
      gfx_gap (10);
      gfx_pos (left - 3, gfx_y (), GFX_R | GFX_T);
      gfx_icon (right);
      gfx_pos (left, gfx_y (), GFX_L | GFX_T | GFX_H);
      gfx_text (2, "Start AP");
   }
   const char *r;
   if (revk_shutting_down (&r))
   {
      gfx_pos (100, 199 - margin, GFX_C | GFX_B);
      gfx_text (-1, "%s", r);
   }
}

void
menu_timezone (struct tm *t, char key)
{                               //  Timezone
   if (key == 'L')
   {
      menu2 = 0;
      return;
   }
   gfx_menu (t, "Timezone");
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
   const char *r;
   if (revk_shutting_down (&r))
   {
      gfx_gap (5);
      gfx_text (-2, "Upgrading");
      gfx_gap (5);
      gfx_text (-1, r);
      return;
   }
   if (percent >= 0)
      return;
   r = revk_command ("upgrade", NULL);
   if (r)
   {
      gfx_gap (5);
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
menu_info (struct tm *t, char key)
{                               //  Info
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
   gfx_iconq (wifi, !revk_link_down ());
   gfx_gap (5);
   gfx_iconq (mqtt, lwmqtt_connected (revk_mqtt (0)));
}

menulist_t list_main[] = {
   {menu_face, "Face select"},
   {menu_wifi, "WiFi settings"},
   {menu_flip, "Flip display"},
   {menu_turn, "Turn display"},
   {menu_timezone, "Timezone"},
   {menu_upgrade, "Upgrade"},
   {menu_mqtt, "MQTT connect"},
   {menu_info, "Info"},
};

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
      {                         // Quick upgrade
         menu1 = 6;
         menu2 = 0xFF;
      }
      if (key == 'L')
      {                         // Quick info
         menu1 = 8;
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
         list_main[menu1 - 1].fun (t, key);
   }
   if (key)
      process (key);
   process (0);
   gfx_unlock ();               // Always safe to extra unlock
}
