// Watch faces

#include "face.h"
#include "menu.h"
#include "iec18004.h"
#include <math.h>

const uint8_t gfx_cos[256] =
   { 255, 255, 255, 255, 255, 255, 254, 254, 253, 252, 252, 251, 250, 249, 248, 247, 246, 245, 243, 242, 240, 239, 237, 236, 234,
   232, 230, 228, 226, 224, 222, 220, 218, 216, 213, 211, 209, 206, 204, 201, 199, 196, 193, 191, 188, 185, 182, 179, 176, 174, 171,
   168, 165, 162, 159,
   156, 152, 149, 146, 143, 140, 137, 134, 131, 127, 124, 121, 118, 115, 112, 109, 106, 103, 99, 96, 93, 90, 87, 84, 81, 79, 76, 73,
   70, 67, 64, 62, 59,
   56, 54, 51, 49, 46, 44, 42, 39, 37, 35, 33, 31, 29, 27, 25, 23, 21, 19, 18, 16, 15, 13, 12, 10, 9, 8, 7, 6, 5, 4, 3, 3, 2, 1, 1,
   0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 1, 1, 2, 3, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 15, 16, 18, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 42, 44, 46, 49,
   51, 54, 56, 59, 62,
   64, 67, 70, 73, 76, 79, 81, 84, 87, 90, 93, 96, 99, 103, 106, 109, 112, 115, 118, 121, 124, 127, 131, 134, 137, 140, 143, 146,
   149, 152, 156, 159, 162,
   165, 168, 171, 174, 176, 179, 182, 185, 188, 191, 193, 196, 199, 201, 204, 206, 209, 211, 213, 216, 218, 220, 222, 224, 226, 228,
   230, 232, 234, 236,
   237, 239, 240, 242, 243, 245, 246, 247, 248, 249, 250, 251, 252, 252, 253, 254, 254, 255, 255, 255, 255, 255
};

const char *
st (uint8_t n)
{
   if (n % 100 < 10 || n % 100 > 20)
   {
      if (n % 10 == 1)
         return "st";
      if (n % 10 == 2)
         return "nd";
      if (n % 10 == 3)
         return "rd";
   }
   return "th";
}

#define PI      3.1415926535897932384626433832795029L
#define sinld(a)        sinl(PI*(a)/180.0L)

static time_t
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

static int
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
gfx_phase (uint8_t cx, uint8_t cy, uint8_t r)
{                               // Show phase
   // TODO moon needs inverting and lighting reversed for southern hemisphere
   inline gfx_pos_t ax (gfx_pos_t a, gfx_pos_t l)
   {
      return cx + l * (gfx_cos[(a + 192) & 255] - 128) / 127;
   }
   inline gfx_pos_t ay (gfx_pos_t a, gfx_pos_t l)
   {
      return cy - l * (gfx_cos[(a) & 255] - 128) / 127;
   }
   for (int a = 0; a < 256; a += 4)
      gfx_line (ax (a, r), ay (a, r), ax (a + 4, r), ay (a + 4, r), 255);       // Outline
   int8_t l = (gfx_cos[moon_phase] - 128) * r / 127;
   gfx_pos_t y = cy - r;
   if (moon_phase > 128)
      for (int a = 255; a >= 128; a--)
      {                         // Light on right
         gfx_pos_t q = ay (a, r);
         while (y < q)
         {
            gfx_pos_t x = cx + (int) l * (gfx_cos[(a + 192) & 255] - 128) / 127;
            gfx_line (ax (a, r), y, x, y, 255);
            y++;
         }
   } else if (moon_phase && moon_phase < 128)
      for (int a = 0; a < 128; a++)
      {                         // Light on left
         gfx_pos_t q = ay (a, r);
         while (y < q)
         {
            gfx_pos_t x = cx + (int) l * (gfx_cos[(a + 192) & 255] - 128) / 127;
            gfx_line (x, y, ax (a, r), y, 255);
            y++;
         }
      }
}

void
gfx_analogue (uint8_t cx, uint8_t cy, uint8_t r, struct tm *t)
{
   inline gfx_pos_t ax (gfx_pos_t a, gfx_pos_t l)
   {
      return cx + l * ((int) gfx_cos[(a + 192) & 255] - 128) / 127;
   }
   inline gfx_pos_t ay (gfx_pos_t a, gfx_pos_t l)
   {
      return cy - l * ((int) gfx_cos[(a) & 255] - 128) / 127;
   }
   for (int a = 0; a < 256; a += 4)
      gfx_line (ax (a, r), ay (a, r), ax (a + 4, r), ay (a + 4, r), 255);
   for (int h = 0; h < 12; h++)
      gfx_line (ax (h * 256 / 12, r), ay (h * 256 / 12, r), ax (h * 256 / 12, (h % 3) ? (int) r * 9 / 10 : (int) r * 8 / 10),
                ay (h * 256 / 12, (h % 3) ? (int) r * 9 / 10 : (int) r * 8 / 10), 255);
   gfx_line (cx, cy, ax (t->tm_min * 256 / 60, (int) r * 95 / 100), ay (t->tm_min * 256 / 60, (int) r * 95 / 100), 255);
   int h = ((int) t->tm_hour * 60 + t->tm_min) * 256 / 12 / 60;
   if (r < 40)
      gfx_line (cx, cy, ax (h, (int) r * 6 / 10), ay (h, (int) r * 6 / 10), 255);
   else
      for (int dx = -1; dx < 1; dx++)
         for (int dy = -1; dy < 1; dy++)
            gfx_line (cx + dx, cy + dy, ax (h, (int) r * 6 / 10) + dx, ay (h, (int) r * 6 / 10) + dy, 255);
}

void
gfx_gap (int8_t g)
{
   gfx_pos (gfx_x (), gfx_y () + g, gfx_a ());
}

void
gfx_status (void)
{                               // shutdown status
   const char *r;
   if (!revk_shutting_down (&r))
      return;
   gfx_text (-1, "%s", r);
}

void
gfx_battery (void)
{
   if (battery >= 80)
      gfx_icon (bat4);
   else if (battery >= 60)
      gfx_icon (bat3);
   else if (battery >= 40)
      gfx_icon (bat2);
   else if (battery >= 20)
      gfx_icon (bat1);
   else
      gfx_icon (bat0);
}

void
gfx_percent (void)
{
   gfx_pos (gfx_x () - 2, gfx_y (), gfx_a ());  // 2 1/2 digits, so move left a tad
   gfx_7seg (1, "%3d", battery);
   gfx_pos (gfx_x () + 2, gfx_y (), gfx_a ());
}

void
gfx_charging (void)
{
   gfx_iconq (charging, bits.charging);
}

void
gfx_wifi (void)
{
   if (!bits.revkstarted)
      return;
   gfx_iconq (wifi, !revk_link_down ());
}

void
gfx_mqtt (void)
{
   if (!bits.revkstarted)
      return;
   gfx_iconq (mqtt, lwmqtt_connected (revk_mqtt (0)));
}

uint16_t
gfx_square_icon_size (uint16_t bytes)
{                               // Size of icon from sizeof(), assuming square
   for (int i = 25; i >= 1; i--)
      if (bytes > i * i * 8)
      {
         bytes /= i + 1;
         break;
      }
   return bytes;
}

void
gfx_square_icon (const uint8_t * icon, uint16_t bytes, uint8_t visible)
{                               // Assumes square icon
   bytes = gfx_square_icon_size (bytes);
   gfx_icon2 (bytes, bytes, visible ? icon : NULL);
}

const char *
gfx_qr (const char *value, uint8_t scale)
{                               // QR code
#ifndef CONFIG_GFX_NONE
   int W = gfx_width ();
   int H = gfx_height ();
   unsigned int width = 0;
 uint8_t *qr = qr_encode (strlen (value), value, widthp: &width, noquiet:1);
   if (qr && width <= W && width <= H)
   {
      const int w = W > H ? H : W;
      int s = w / width;
      if (scale)
         s = scale;
      gfx_pos_t ox,
        oy;
      gfx_draw (width * s, width * s, 0, 0, &ox, &oy);
      for (gfx_pos_t y = 0; y < width; y++)
         for (gfx_pos_t x = 0; x < width; x++)
            if (qr[width * y + x] & QR_TAG_BLACK)
               for (gfx_pos_t dy = 0; dy < s; dy++)
                  for (gfx_pos_t dx = 0; dx < s; dx++)
                     gfx_pixel (ox + x * s + dx, oy + y * s + dy, 0xFF);
   }
   if (!qr)
      return "QR failed";
   free (qr);
   if (width > W || width > H)
      return "Too big";
#endif
   return NULL;
}

void
face_init (void)
{                               // Initial watch face
   gfx_lock ();
   gfx_refresh ();
   gfx_clear (0);
   gfx_pos (100, 100, GFX_C | GFX_M);
   gfx_qr ("HTTPS://WATCHY.REVK.UK", 0);
   gfx_unlock ();
}

typedef void face_t (struct tm *);
#define face(name,description)	extern face_t face_##name;
#include "faces.m"
face_t *const faces[] = {
#define face(name,description)	&face_##name,
#include "faces.m"
};

void
face_low_battery (struct tm *t)
{
   gfx_pos (100, 0, GFX_C | GFX_T | GFX_V);
   gfx_text (6, "LOW");
   gfx_text (4, "BATTERY");
   gfx_pos (100, 199, GFX_C | GFX_B | GFX_V);
   char temp[30];
   strftime (temp, sizeof (temp), "%F", t);
   gfx_7seg (3, "%s", temp);
   gfx_gap(-5);
   strftime (temp, sizeof (temp), "%H:%M", t);
   gfx_7seg (7, "%s", temp);
}

extern uint8_t face;            // Face number
void
face_show (time_t now, char key)
{
   struct tm t;
   localtime_r (&now, &t);
   if (bits.reset == ESP_RST_BROWNOUT)
   {
      if (key || bits.charging)
         bits.reset = 0;        // normal face
      else
      {
         gfx_lock ();
         gfx_clear (0);
         face_low_battery (&t);
         gfx_unlock ();
         return;
      }
   }
   if (bits.newhour && !t.tm_hour)
   {                            // New day
      last_steps = steps;
      acc_step_reset ();
      steps = 0;
      bits.newday = 1;
   }
   if (bits.newhour || !moon_next)
   {
      time_t now = mktime (&t);
      int cycle = lunarcycle (now);
      time_t base = fullmoon (cycle);
      time_t next = fullmoon (cycle + 1);
      moon_phase = (256 * (now - base) / (next - base));
      moon_next = next;
   }
   if (menu1 || key)
      menu_show (&t, key);
   if (!menu1)
   {
      if (face >= sizeof (faces) / sizeof (*faces))
         face = 0;
      gfx_lock ();
      gfx_clear (0);
      faces[face] (&t);
      gfx_unlock ();
   }
}

void
face_basic (struct tm *t)
{                               // Digital face
   char temp[30];
   gfx_pos (100, 0, GFX_C | GFX_T | GFX_H);
   strftime (temp, sizeof (temp), "%H:%M", t);
   gfx_7seg (8, "%s", temp);
   strftime (temp, sizeof (temp), "%F", t);
   gfx_pos (100, 90, GFX_C | GFX_T | GFX_V);
   gfx_7seg (3, "%s", temp);
   gfx_pos (100, 130, GFX_C | GFX_B);
   gfx_status ();
   gfx_pos (0, 199, GFX_L | GFX_B | GFX_H);
   strftime (temp, sizeof (temp), "%FT%H:%M%z", t);
   gfx_qr (temp, 2);
   gfx_charging ();
   gfx_battery ();
   gfx_percent ();
   gfx_pos (199, 165, GFX_R | GFX_B | GFX_H);
   gfx_7seg (2, "%6d", steps);
   gfx_wifi ();
   gfx_mqtt ();
   strftime (temp, sizeof (temp), "%a", t);
   gfx_pos (199, 199, GFX_R | GFX_B | GFX_H);
   gfx_text (4, "%s", temp);
}

void
face_combined (struct tm *t)
{                               // Combined analogue/digital
   char temp[30];
   gfx_pos (100, 0, GFX_C | GFX_T | GFX_H);
   strftime (temp, sizeof (temp), "%H:%M", t);
   gfx_7seg (8, "%s", temp);
   gfx_pos (199, 199, GFX_R | GFX_B | GFX_V);
   strftime (temp, sizeof (temp), "%a", t);
   gfx_text (3, "%s", temp);
   gfx_gap (-5);
   gfx_7seg (6, "%2d", t->tm_mday);
   strftime (temp, sizeof (temp), "%b", t);
   gfx_text (-3, "%s", temp);
   gfx_pos (5, 80, GFX_L | GFX_T);
   gfx_7seg (2, "%d", steps);
   gfx_pos (115, 199, GFX_C | GFX_B | GFX_V);
   gfx_battery ();
   gfx_percent ();
   gfx_charging ();
   gfx_wifi ();
   gfx_mqtt ();
   gfx_analogue (50, 150, 49, t);
}

void
face_analogue (struct tm *t)
{
   gfx_analogue (100, 100, 99, t);
   char temp[10];
   gfx_pos (150, 100, GFX_C | GFX_M);
   gfx_text (2, "%02d", t->tm_mday);
   gfx_box (26 + 2, 18 + 2, 255);
   strftime (temp, sizeof (temp), "%a", t);
   gfx_pos (50, 100, GFX_C | GFX_M);
   gfx_text (2, "%s", temp);
   gfx_box (38 + 2, 18 + 2, 255);
   gfx_pos (100, 150, GFX_C | GFX_M);
   gfx_7seg (2, "%6d", steps);
   gfx_pos (100, 50, GFX_C | GFX_M);
   gfx_icon (ajk);
   gfx_pos (0, 0, GFX_L | GFX_T);
   gfx_battery ();
   gfx_pos (199, 0, GFX_R | GFX_T);
   gfx_charging ();
   if (bits.revkstarted)
   {
      gfx_pos (199, 199, GFX_R | GFX_B);
      gfx_wifi ();
      gfx_pos (0, 199, GFX_L | GFX_B);
      gfx_mqtt ();
   }
   gfx_pos (100, 130, GFX_C | GFX_B);
   gfx_status ();
}
