// Alteran faces

#include "face.h"

static void
digit (gfx_pos_t s, uint8_t c)
{
   gfx_pos_t x,
     y;
   gfx_draw (c == ':' ? s : s * 3, s * 8, s, s, &x, &y);
   // TODO expand to full text logic and processing a string
   uint16_t m = (c == ':' ? 0x208 : c == '0' ? 0x16F : (1 << (c - '0')) - 1);
   gfx_pos_t nx = gfx_x ();
   gfx_pos_t ny = gfx_y ();
   gfx_pos_t na = gfx_a ();
   // TODO this could go 3x4 for letters maybe
   for (int row = 0; row < 4; row++)
      for (int col = 0; col < 3; col++)
      {
         if (m & 1)
            for (int dx = 0; dx < s; dx++)
               for (int dy = 0; dy < s * 2; dy++)
                  gfx_pixel (x + col * s + dx, y + row * s * 2 + dy, 0xFF);
         m >>= 1;
      }
   if (c >= '0' && c <= '9')
   {                            // This is only for digits
      m = 0x17;
      for (int row = 0; row < 3; row++)
         for (int col = 0; col < 3; col++)
         {
            if (m & 1)
               for (int dx = 0; dx < s; dx++)
                  for (int dy = 0; dy < s; dy++)
                     gfx_pixel (x + col * s + dx, y + 3 * s * 2 + row * s + dy, 0xFF);
            m >>= 1;
         }
   }
   gfx_pos (nx, ny, na);
}

void
face_alteran (struct tm *t)
{
   char temp[30];
   gfx_pos (0, 0, GFX_T | GFX_L | GFX_H);
   digit (12, '0' + t->tm_hour / 10);
   digit (12, '0' + t->tm_hour % 10);
   digit (12, ':');
   digit (12, '0' + t->tm_min / 10);
   digit (12, '0' + t->tm_min % 10);
   gfx_pos (0, 100, GFX_L | GFX_T | GFX_V);
   strftime (temp, sizeof (temp), "%a", t);
   gfx_text (3, "%s", temp);
   gfx_gap (3);
   gfx_text (5, "%2d", t->tm_mday);
   strftime (temp, sizeof (temp), "%b", t);
   gfx_text (-3, "%s", temp);
   gfx_pos (199, 100, GFX_R | GFX_T | GFX_V);
   gfx_text (2, "%4d", t->tm_year + 1900);
   gfx_gap (10);
   gfx_text (2, "%5d", steps - stepbase);
   gfx_pos (199, 199, GFX_B | GFX_R | GFX_V);
   gfx_battery ();
   gfx_percent ();
   gfx_charging ();
   gfx_pos (180, 199, GFX_B | GFX_R | GFX_V);
   gfx_wifi ();
   gfx_mqtt ();
}
