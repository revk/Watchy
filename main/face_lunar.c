// Luna faces

#include "face.h"

static void
lunar (struct tm *t, uint8_t force)
{
   char temp[30];
   gfx_pos (199, 0, GFX_R | GFX_T | GFX_V);
   gfx_7seg (6, "%02d", t->tm_hour);
   gfx_7seg (4, "%02d", t->tm_min);
   gfx_gap (5);
   strftime (temp, sizeof (temp), "%a", t);
   gfx_text (3, temp);
   gfx_gap (5);
   gfx_pos (0, 100 + 5, GFX_L | GFX_T | GFX_V);
   gfx_7seg (2, "%-5d", steps);
   gfx_pos (gfx_x (), gfx_y () + 2, GFX_L | GFX_T | GFX_H);
   gfx_battery ();
   gfx_charging ();
   gfx_wifi ();
   gfx_mqtt ();
   strftime (temp, sizeof (temp), "%F", t);
   gfx_pos (100, 199, GFX_C | GFX_B | GFX_V);
   gfx_7seg (3, temp);
   {
      struct tm m;
      localtime_r (&moon_next, &m);
      strftime (temp, sizeof (temp), "%H:%M", &m);
      gfx_text (-2, "Next: %d%s, %s", m.tm_mday, st (m.tm_mday), temp);
   }
   gfx_status ();
   {
      const char *r;
      if (revk_shutting_down (&r))
      {
         gfx_pos (100, 170, GFX_C | GFX_B);
         gfx_text (-1, "%s", r);
      }
   }
   // Show phase
   // TODO moon needs inverting and lighting reversed for southern hemisphere
   const int r = 48;
   inline gfx_pos_t ax (gfx_pos_t a, gfx_pos_t l)
   {
      return 50 + l * (gfx_cos[(a + 192) & 255] - 128) / 127;
   }
   inline gfx_pos_t ay (gfx_pos_t a, gfx_pos_t l)
   {
      return 50 - l * (gfx_cos[(a) & 255] - 128) / 127;
   }
   gfx_pos (0, 0, GFX_L | GFX_T);
   if (force)
      gfx_icon (deathstar);
   else
      gfx_icon (moon);
   for (int a = 0; a < 256; a += 4)
      gfx_line (ax (a, r), ay (a, r), ax (a + 4, r), ay (a + 4, r), 255);       // Outline
   int8_t l = (gfx_cos[moon_phase] - 128) * r / 127;
   gfx_pos_t y = 50 - r;
   if (moon_phase > 128)
      for (int a = 255; a >= 128; a--)
      {                         // Light on right
         gfx_pos_t q = ay (a, r);
         while (y < q)
         {
            gfx_pos_t x = 50 + (int) l * (gfx_cos[(a + 192) & 255] - 128) / 127;
            if (!force || (y & 1))
               gfx_line (ax (a, r), y, x, y, 255);
            y++;
         }
   } else if (moon_phase && moon_phase < 128)
      for (int a = 0; a < 128; a++)
      {                         // Light on left
         gfx_pos_t q = ay (a, r);
         while (y < q)
         {
            gfx_pos_t x = 50 + (int) l * (gfx_cos[(a + 192) & 255] - 128) / 127;
            if (!force || (y & 1))
               gfx_line (x, y, ax (a, r), y, 255);
            y++;
         }
      }
}

void
face_lunar (struct tm *t)
{
   lunar (t, 0);
}

void
face_deathstar (struct tm *t)
{
   lunar (t, 1);
}
