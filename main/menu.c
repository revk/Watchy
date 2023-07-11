// Menus

static const char __attribute__((unused)) TAG[] = "Menu";

#include "revk.h"
#include "watchy.h"
#include "gfx.h"
#include "icons.h"
#include "menu.h"

RTC_NOINIT_ATTR uint8_t btnlast;

typedef void menu_fun (struct tm *, char);
typedef struct
{
   menu_fun *fun;
   const char *name;
} menulist_t;

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
   // Arrows

   // Time
   char temp[30];
   strftime (temp, sizeof (temp), "%F", t);
   gfx_pos (100, 0, GFX_C | GFX_T);
   gfx_text (-2, title ? : temp);
   strftime (temp, sizeof (temp), "%H:%M %Z", t);
   gfx_pos (100, 199, GFX_C | GFX_B);
   gfx_text (2, temp);
   gfx_pos (0, 0, GFX_L | GFX_T);
   gfx_icon2 (32, 32, icon_up);
   gfx_pos (199, 0, GFX_R | GFX_T);
   gfx_icon2 (32, 32, icon_right);
   gfx_pos (0, 199, GFX_L | GFX_B);
   gfx_icon2 (32, 32, icon_down);
   gfx_pos (199, 199, GFX_R | GFX_B);
   gfx_icon2 (32, 32, icon_left);
   gfx_pos (100, 24, GFX_C | GFX_T | GFX_V);
}

uint8_t
menu_list (struct tm *t, uint8_t pos, uint8_t len, menulist_t * m, const char *title, char key)
{                               // Show a list, handle key U/D/L, return new position
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
   const int margin = 24;
   const int left = 24;
   const int line = 22;
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
         gfx_pos (18 - 32, y, GFX_L | GFX_T);
         gfx_icon2 (32, 32, icon_right);
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
   bits.wifi = 1;
   bits.holdoff = 1;
   gfx_menu (t, "WiFi");
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
   if (key == 'L')
   {
      menu2 = 0;
      bits.wifi=0;
      bits.holdoff = 0;
      return;
   }
   bits.wifi = 1;
   bits.holdoff = 1;
   gfx_menu (t, "Upgrade");
   extern const char *otahost;
   gfx_gap (5);
   gfx_text (-2, "from...");
   gfx_text (strlen (otahost) > 16 ? -1 : -2, otahost);
   if (revk_link_down ())
   {
      gfx_gap (5);
      gfx_text (-2, "Waiting");
      return;
   }
      gfx_gap (5);
      int8_t percent=revk_ota_progress();
      if(percent==-2)gfx_text(-2,"Up to date");
      else if(percent==101)gfx_text(-2,"Done");
      else if(percent>=0&&percent<=100)gfx_text(5,"%3d%%",percent);
   const char *r;
   if (revk_shutting_down (&r))
   {
      ESP_LOGE (TAG, "Shutting down %s", r);
      gfx_gap (5);
      gfx_text (-2, "Upgrading");
      gfx_gap (5);
      gfx_text (-1, r);
      return;
   }
   ESP_LOGE (TAG, "Start upgrade");
   r = revk_command ("upgrade", NULL);
   if (r)
   {
      gfx_gap (5);
      gfx_text (-2, "Trying");
      gfx_gap (5);
      gfx_text (-1, r);
      return;
   }
   if (uptime () > 120 || key == 'R')
   {
      menu1 = 0;
      bits.wifi=0;
      bits.holdoff = 0;
      return;
   }
}

void
menu_face (struct tm *t, char key)
{                               //  Face select
   if (key == 'L')
   {
      menu2 = 0;
      return;
   }
   gfx_menu (t, "Face");
}

void
menu_flip (struct tm *t, char key)
{                               //  Flip display
   bits.holdoff = 1;
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
   bits.holdoff = 1;
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
   gfx_menu (t, "Info");
}

menulist_t list_main[] = {
   {menu_face, "Face select"},
   {menu_wifi, "WiFi settings"},
   {menu_flip, "Flip display"},
   {menu_turn, "Turn display"},
   {menu_timezone, "Timezone"},
   {menu_upgrade, "Upgrade"},
   {menu_info, "Info"},
};


void
menu_main (struct tm *t, char key)
{
   if (key == 'R')
      menu2 = 1;                // Selected
   else
      menu1 = menu_list (t, menu1, sizeof (list_main) / sizeof (*list_main), list_main, NULL, key);
}

void
menu_show (struct tm *t, char key)
{
   ESP_LOGE (TAG, "Menu %d %d %d key %c flip %d", menu1, menu2, menu3, key, flip);
   if (key && !menu1)
   {
      menu1 = 1;                // Enter menu
      menu2 = 0;
      menu3 = 0;
      key = 0;                  // used the key top enter menu
   }
   // Menu functions called with key set to update state and then called (after state change) with no key to display
   void process (char key)
   {
      if (!menu1)
         return;
      if (!menu2)
         menu_main (t, key);
      else if (!menu3 && menu1 && menu1 <= sizeof (list_main) / sizeof (*list_main))
         list_main[menu1 - 1].fun (t, key);
   }
   if (key)
      process (key);
   process (0);
   gfx_unlock ();               // Always safe to extra unlock
}
