// Luna faces

#include "face.h"
#include "math.h"

#define PI      3.1415926535897932384626433832795029L
#define sinld(a)        sinl(PI*(a)/180.0L)

time_t
fullmoon (int cycle)
{                               // report full moon for specific lunar cycle
   long double k = cycle + 0.5;
   long double T = k / 1236.85L;
   long double JD =
      2415020.75933L + 29.53058868L * k + 0.0001178L * T * T - 0.000000155L * T * T * T +
      0.00033L * sinld (166.56L + 132.87L * T - 0.009173L * T * T);
   long double M = 359.2242L + 29.10535608L * k - 0.0000333L * T * T - 0.00000347L * T * T * T;
   long double M1 = 306.0253L + 385.81691806L * k + 0.0107306L * T * T + 0.00001236L * T * T * T;
   long double F = 21.2964L + 390.67050646L * k - 0.0016528L * T * T - 0.00000239L * T * T * T;
   long double A = (0.1734 - 0.000393 * T) * sinld (M)  //
      + 0.0021 * sinld (2 * M)  //
      - 0.4068 * sinld (M1)     //
      + 0.0161 * sinld (2 * M1) //
      - 0.0004 * sinld (3 * M1) //
      + 0.0104 * sinld (2 * F)  //
      - 0.0051 * sinld (M + M1) //
      - 0.0074 * sinld (M - M1) //
      + 0.0004 * sinld (2 * F + M)      //
      - 0.0004 * sinld (2 * F - M)      //
      - 0.0006 * sinld (2 * F + M1)     //
      + 0.0010 * sinld (2 * F - M1)     //
      + 0.0005 * sinld (M + 2 * M1);    //
   JD += A;
   return (JD - 2440587.5L) * 86400LL;
}

int
lunarcycle (time_t t)
{                               // report cycle for previous full moon
   int cycle = ((long double) t + 2207726238UL) / 2551442.86195200L;    // Guess
   time_t f = fullmoon (cycle);
   if (t < f)
      return cycle - 1;
   f = fullmoon (cycle + 1);
   if (t >= f)
      return cycle + 1;
   return cycle;
}

void
face_lunar (struct tm *t)
{
   if (bits.newhour)
   {
      time_t now = mktime (t);
      int cycle = lunarcycle (now);
      time_t base = fullmoon (cycle);
      time_t next = fullmoon (cycle + 1);
      moon_phase = (256 * (now - base) / (next - base));
      moon_next = next;
   }
   char temp[30];
   gfx_pos (199, 0, GFX_R | GFX_T | GFX_V);
   gfx_7seg (6, "%02d", t->tm_hour);
   gfx_7seg (4, "%02d", t->tm_min);
   gfx_gap (5);
   gfx_pos (gfx_x (), gfx_y (), GFX_R | GFX_T | GFX_H);
   gfx_battery ();
   gfx_charging ();
   gfx_wifi ();
   gfx_mqtt ();
   gfx_pos (0, 100, GFX_L | GFX_T | GFX_V);
   gfx_7seg (2, "%-5d", steps);
   strftime (temp, sizeof (temp), "%F", t);
   gfx_pos (100, 199, GFX_C | GFX_B | GFX_V);
   gfx_7seg (3, temp);
   {
      struct tm m;
      localtime_r (&moon_next, &m);
      strftime (temp, sizeof (temp), "%F %H:%M", &m);
      gfx_gap (-2);
      gfx_7seg (2, temp);
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
            gfx_line (x, y, ax (a, r), y, 255);
            y++;
         }
      }
}
