// Menus

static const char __attribute__((unused)) TAG[] = "Menu";

#include "revk.h"
#include "watchy.h"
#include "gfx.h"
#include "icons.h"
#include "menu.h"

RTC_NOINIT_ATTR uint8_t btnlast;

typedef void menu_fun (struct tm *t);
typedef struct
{
   menu_fun *fun;
   const char *name;
} menulist_t;

void
menu_upgrade (struct tm *t)
{                               //  Upgrade
}

menulist_t mainmenu[] = {
   {menu_upgrade, "Upgrade"},
};

void
gfx_menu (struct tm *t)
{                               // Start menu
   gfx_lock ();
   gfx_clear (0);
   // Arrows

   // Time
   char temp[30];
   strftime (temp, sizeof (temp), "%F", t);
   gfx_pos (100, 0, GFX_C | GFX_T);
   gfx_text (2, temp);
   strftime (temp, sizeof (temp), "%H:%M %Z", t);
   gfx_pos (100, 199, GFX_C | GFX_B);
   gfx_text (2, temp);
   gfx_pos (0, 0, GFX_L | GFX_T);
   gfx_icon2 (32, 32, icon_power);
   gfx_pos (199, 0, GFX_R | GFX_T);
   gfx_icon2 (32, 32, icon_power);
   gfx_pos (0, 199, GFX_L | GFX_B);
   gfx_icon2 (32, 32, icon_power);
   gfx_pos (199, 199, GFX_R | GFX_B);
   gfx_icon2 (32, 32, icon_power);
}

void
menu_list (struct tm *t, uint8_t pos, uint8_t len, menulist_t * m)
{
   gfx_menu (t);

}

void
menu_show (struct tm *t)
{
   uint8_t btn = bits.buttons & ~btnlast;       // Pushed
   btnlast = bits.buttons;
   if (bits.buttons && bits.buttons != BTNCANCEL && !menu1)
   {                            // Any button starting, even if not seen as rising edge
      menu1 = 1;
      menu2 = 0;
      menu3 = 0;
   }
   ESP_LOGI (TAG, "Menu %d %d %d btns %X", menu1, menu2, menu3, btn);
   if (btn == BTNCANCEL)
   {
      if (menu3)
         menu3 = 0;
      else if (menu2)
         menu2 = 0;
      else
         menu1 = 0;
   }
   if (btn == BTNSELECT)
   {
      if (menu1 & !menu2)
         menu2 = 1;
      else if (menu2 && !menu3)
         menu3 = 1;
   }
   if (!menu1)
      return;
   if (!menu2)
   {                            // Showing top level menu
      menu_list (t, menu1, sizeof (mainmenu) / sizeof (*mainmenu), mainmenu);


   }
   gfx_unlock ();               // Always safe to extra unlock
}
