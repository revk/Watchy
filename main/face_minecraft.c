// Luna faces

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
   extern const uint8_t gfx_font0[];
   const uint8_t *f = gfx_font0 + (c - ' ') * 5;
   for (uint8_t r = 0; r < 5; r++)
      for (uint8_t c = 0; c < 3; c++)
         if (f[r] & (1 << c))
         {
            gfx_pos (x + c * s, y + r * s, 0);
            gfx_icon (minecraft);
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
   gfx_gap (5);
   gfx_blocky (5, "%2d", t->tm_mday);
   strftime (temp, sizeof (temp), "%b", t);
   gfx_blocky (-3, "%s", temp);
   gfx_gap (5);
   gfx_blocky (2, "%5d", steps);
   gfx_pos (199, 199, GFX_B | GFX_R | GFX_H);
   gfx_battery ();
   gfx_charging ();
   gfx_pos (199, 180, GFX_B | GFX_R | GFX_H);
   gfx_wifi ();
   gfx_mqtt ();
}
