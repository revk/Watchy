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
gfx_menu (struct tm *t, const char *title)
{                               // Start menu
   gfx_lock ();
   gfx_clear (0);
   // Arrows

   // Time
   char temp[30];
   strftime (temp, sizeof (temp), "%F", t);
   gfx_pos (100, 0, GFX_C | GFX_T);
   gfx_text (2, title ? : temp);
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
   return pos;
}

void
menu_wifi (struct tm *t, char key)
{                               //  WiFi settings and AP mode
}

void
menu_timezone (struct tm *t, char key)
{                               //  Timezone
}

void
menu_upgrade (struct tm *t, char key)
{                               //  Upgrade
}

void
menu_face (struct tm *t, char key)
{                               //  Face select
}

void
menu_flip (struct tm *t, char key)
{                               //  Flip display
}

void
menu_turn (struct tm *t, char key)
{                               //  Turn display
}

void
menu_info (struct tm *t, char key)
{                               //  Info
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
   if (key == 'L')
      menu1 = 0;
   else
      menu1 = menu_list (t, menu1, sizeof (list_main) / sizeof (*list_main), list_main, NULL, key);
}

void
menu_show (struct tm *t, char key)
{
	if(key&&!menu1)menu1=1; // Enter menu
   ESP_LOGI (TAG, "Menu %d %d %d key %c", menu1, menu2, menu3, key);
   // Menu functions called with key set to update state and then called (after state change) with no key to display
   if (!menu1)
      return;
   if (key)
   {
      if (!menu2)
         menu_main (t, key);
      else if (!menu3 && menu1 && menu1 <= sizeof (list_main) / sizeof (*list_main))
         list_main[menu1 - 1].fun(t, key);
   }
   if (!menu2)
      menu_main (t, 0);
   else if (!menu3 && menu1 && menu1 <= sizeof (list_main) / sizeof (*list_main))
      list_main[menu1 - 1].fun(t, 0);
   gfx_unlock ();               // Always safe to extra unlock
}
