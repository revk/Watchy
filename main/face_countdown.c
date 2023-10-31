// Countdown face

#include "face.h"

void
face_countdown (struct tm *t)
{
   char temp[30];
   gfx_pos (100, 0, GFX_C | GFX_T | GFX_H);
   strftime (temp, sizeof (temp), "%H:%M", t);
   gfx_7seg (8, "%s", temp);

   if (t->tm_year > 100)
   {
      int y = 0,
         m = 0,
         d = 0,
         H = 0,
         M = 0;
      sscanf (rtcdeadline, "%d-%d-%d %d:%d", &y, &m, &d, &H, &M);
      if (!m)
         m = 1;
      if (!d)
         d = 1;
      if (!y)
      {                         // Regular date
         y = t->tm_year + 1900;
         if (t->tm_mon * 2678400 + t->tm_mday * 86400 + t->tm_hour * 3600 + t->tm_min * 60 + t->tm_sec >
             (m - 1) * 2678400 + d * 86400 + H * 3600 + M * 60)
            y++;
      }
      // Somewhat convoluted to allow for clock changes
      int days = 0;
      {                         // Work out days (using H:M:S non DST)
       struct tm target = { tm_year: y - 1900, tm_mon: m - 1, tm_mday:d };
       struct tm today = { tm_year: t->tm_year, tm_mon: t->tm_mon, tm_mday:t->tm_mday };
         days = (mktime (&target) - mktime (&today)) / 86400;
         if (t->tm_hour * 3600 + t->tm_min * 60 + t->tm_sec > H * 3600 + M * 60)
            days--;             // Passed current time
      }
    struct tm deadt = { tm_year: y - 1900, tm_mon: m - 1, tm_mday: d - days, tm_hour: H, tm_min: M, tm_isdst:-1 };
      t->tm_sec = 0;             // Whole minutes
      int seconds = mktime (&deadt) - mktime (t);
      if (days < 0)
         days = seconds = 0;    // Deadline reached
      if (days > 999)
         days = 999;

      gfx_pos (199, 199, GFX_B | GFX_R | GFX_V);
      gfx_7seg (4, "%3d %02d:%02d", days, seconds / 3600, seconds / 60 % 60, seconds % 60);

      gfx_pos (0, 140, GFX_L | GFX_M | GFX_V);
      if (m == 12 && d == 25)
         gfx_text (4, "XMAS");
   }
   strftime (temp, sizeof (temp), "%F", t);
   gfx_pos (100, 90, GFX_C | GFX_T | GFX_V);
   gfx_7seg (3, "%s", temp);

   gfx_pos (199, 140, GFX_R | GFX_M | GFX_H);
   gfx_battery ();
   gfx_percent ();
   gfx_charging ();
   gfx_wifi ();
   gfx_mqtt ();
}
