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
   gfx_7seg (2, "%-5d", steps - stepbase);
   gfx_pos (gfx_x (), gfx_y () + 11, GFX_L | GFX_M | GFX_H);
   gfx_battery ();
   gfx_percent ();
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
   gfx_pos (0, 0, GFX_L | GFX_T);
   if (force)
      gfx_icon (deathstar);
   else
      gfx_icon (moon);
   gfx_phase (50, 50, 48);
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
