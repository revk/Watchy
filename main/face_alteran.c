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
   {
      digit (s, *t++);
      if (s >= 10)
         gfx_pos (gfx_x () - 1, gfx_y (), gfx_a ());
   }
}

static const unsigned char *sg[] = {
   icon_sg01,
   icon_sg02,
   icon_sg03,
   icon_sg04,
   icon_sg05,
   icon_sg06,
   icon_sg07,
   icon_sg08,
   icon_sg09,
   icon_sg10,
   icon_sg11,
   icon_sg12,
   icon_sg13,
   icon_sg14,
   icon_sg15,
   icon_sg16,
   icon_sg17,
   icon_sg18,
   icon_sg19,
   icon_sg20,
   icon_sg21,
   icon_sg22,
   icon_sg23,
   icon_sg24,
   icon_sg25,
   icon_sg26,
   icon_sg27,
   icon_sg28,
   icon_sg29,
   icon_sg30,
   icon_sg31,
   icon_sg32,
   icon_sg33,
   icon_sg34,
   icon_sg35,
   icon_sg36,
   icon_sg37,
   icon_sg38,
   icon_sg39,
   icon_sg40,
};

static void
icon_sg (int n)
{
   if (n >= sizeof (sg) / sizeof (*sg))
      return;
   gfx_square_icon (sg[n], icon_sg01_size, 1);

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
   // Random gate address
   gfx_pos (0, 150, GFX_L | GFX_B | GFX_H);
   unsigned long long v = 0;
   esp_fill_random (&v, sizeof (v));
   unsigned long long picked = 0;
   for (int i = 0; i < 6; i++)
   {
      unsigned long long d = v % (38 - i);
      v /= (38 - i);
      for (int c = 0; c <= d; c++)
         if (picked & (1ULL << c))
            d++;
      picked |= (1ULL << d);
      icon_sg (d + 1);
      gfx_pos (gfx_x () + 1, gfx_y (), gfx_a ());
   }
   gfx_icon (sg01big);
}
