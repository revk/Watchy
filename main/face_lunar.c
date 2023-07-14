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
   int cycle = ((long double) t + 2207726238UL) / 2551442.86195200L;
   time_t f = fullmoon (cycle);
   if (t < f)
      return cycle - 1;
   f = fullmoon (cycle + 1);
   if (t >= f)
      return cycle + 1;
   return cycle;
}

int
lunarphase (time_t t)
{
   int cycle = lunarcycle (t);
   time_t base = fullmoon (cycle);
   time_t next = fullmoon (cycle + 1);
   return (360 * (t - base) / (next - base));
}

void
face_lunar (struct tm *t)
{
   char temp[30];
   gfx_pos (0, 0, GFX_L | GFX_T);
   gfx_icon (moon);
   gfx_pos (199, 0, GFX_R | GFX_T | GFX_V);
   gfx_7seg (6, "%02d", t->tm_hour);
   gfx_7seg (4, "%02d", t->tm_min);
   gfx_pos (0, 100, GFX_L | GFX_T);
   gfx_7seg (2, "%-5d", steps);
   strftime (temp, sizeof (temp), "%F", t);
   gfx_pos (100, 199, GFX_C | GFX_B);
   gfx_7seg (3, temp);
   const char *r;
   if (revk_shutting_down (&r))
   {
      gfx_pos (100, 170, GFX_C | GFX_B);
      gfx_text (-1, "%s", r);
   }
}
