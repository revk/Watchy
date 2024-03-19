// Alteran faces

#include "face.h"

static void
digit (int s, uint8_t c)
{
   gfx_pos_t x,
     y;
   gfx_draw (c == ':' || c == '.' ? s : s * 3, s * 8, s, s, &x, &y);
   uint16_t m = (c == ' ' ? 0 : c == '-' ? 0x1C0 : c == ':' ? 0x208 : c == '0' ? 0x16F : (1 << (c - '0')) - 1);
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

static void
digits (int s, const char *t)
{
   while (*t)
      digit (s, *t++);
}

void
face_alteran (struct tm *t)
{
   char temp[30];
   gfx_pos (0, 0, GFX_T | GFX_L | GFX_H);
   strftime (temp, sizeof (temp), "%H:%M", t);
   digits (12, temp);
   gfx_pos (0, 199, GFX_L | GFX_B | GFX_H);
   strftime (temp, sizeof (temp), "%F", t);
   digits (4, temp);
   gfx_pos (199, 180, GFX_R | GFX_B | GFX_H);
   sprintf (temp, "%05ld", steps - stepbase);
   digits (2, temp);
   gfx_pos (199, 199, GFX_R | GFX_B | GFX_H);
   gfx_battery ();
   gfx_wifi ();
   gfx_pos(0,160,GFX_L|GFX_B|GFX_H);
   gfx_icon(sg01);
   gfx_icon(sg02);
   gfx_icon(sg03);
   gfx_icon(sg04);
   gfx_icon(sg05);
   gfx_icon(sg06);
   gfx_icon(sg07);
   gfx_icon(sg08);
}
