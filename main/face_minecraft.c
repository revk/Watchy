// Minecraft faces

#include "face.h"

static void
mc (gfx_pos_t s, uint8_t c)
{
   gfx_pos_t x,
     y;
   gfx_draw (s * 3, s * 5, s, s, &x, &y);
   gfx_pos_t nx = gfx_x ();
   gfx_pos_t ny = gfx_y ();
   gfx_pos_t na = gfx_a ();
   uint8_t lx,
     hx,
     ly,
     hy;
   extern uint8_t const *gfx_font_pack0[];
   const uint8_t *data = gfx_font_pack0[c - ' '];
   if (data)
   {
      data = gfx_pack (data, &lx, &hx, &ly, &hy, 8);
      uint8_t d = 0;
      for (gfx_pos_t row = 0; row < 5; row++)
         for (gfx_pos_t col = 0; col < 3; col++)
         {
            if (row >= ly && row < hy && col >= lx && col < hx && !(col & 7))
               d = *data++;
            if (d & 0x80)
            {
               gfx_pos (x + col * s, y + row * s, 0);
               gfx_icon (minecraft);
            }
            d <<= 1;
         }
   }
   gfx_pos (nx, ny, na);
}

void
face_minecraft (struct tm *t)
{
   char temp[30];
   gfx_pos (0, 0, GFX_T | GFX_L | GFX_H);
   mc (18, '0' + t->tm_hour / 10);
   mc (18, '0' + t->tm_hour % 10);
   gfx_pos (0, 199, GFX_B | GFX_L | GFX_H);
   mc (18, '0' + t->tm_min / 10);
   mc (18, '0' + t->tm_min % 10);
   gfx_pos (199, 0, GFX_R | GFX_T | GFX_V);
   strftime (temp, sizeof (temp), "%a", t);
   gfx_blocky (3, "%s", temp);
   gfx_gap (3);
   gfx_blocky (5, "%2d", t->tm_mday);
   strftime (temp, sizeof (temp), "%b", t);
   gfx_blocky (-3, "%s", temp);
   gfx_blocky (2, "%4d", t->tm_year + 1900);
   gfx_gap (10);
   gfx_blocky (2, "%5d", steps - stepbase);
   gfx_pos (199, 199, GFX_B | GFX_R | GFX_V);
   gfx_battery ();
   gfx_percent ();
   gfx_charging ();
   gfx_pos (180, 199, GFX_B | GFX_R | GFX_V);
   gfx_wifi ();
   gfx_mqtt ();
}
