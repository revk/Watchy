// Watch faces

#include "face.h"
#include "menu.h"
#include "iec18004.h"

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

void
gfx_square_icon (const uint8_t * icon, uint16_t bytes, uint8_t visible)
{                               // Assumes square icon
   // Work out size, assuming square, allow up to 200x200
   for (int i = 25; i >= 1; i--)
      if (bytes > i * i * 8)
      {
         bytes /= i + 1;
         break;
      }
   gfx_icon2 (bytes, bytes, icon);
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

extern uint8_t face;            // Face number
void
face_show (time_t now, char key)
{
   struct tm t;
   localtime_r (&now, &t);
   if (bits.newhour && !t.tm_hour)
   {                            // New day
      last_steps = steps;
      acc_step_reset ();
      steps = 0;
      bits.newday = 1;
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
{                               // Basic face
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
   gfx_pos (gfx_x (), gfx_y () - 3, gfx_a ());  // Position for battery icon - this is temporary until calibrated
   gfx_7seg (1, "%3d", battery);
   gfx_pos (199, 165, GFX_R | GFX_B | GFX_H);
   gfx_7seg (2, "%6d", steps);
   strftime (temp, sizeof (temp), "%a", t);
   gfx_wifi ();
   gfx_mqtt ();
   gfx_pos (199, 199, GFX_R | GFX_B | GFX_H);
   gfx_text (4, "%s", temp);
}

void
face_analogue (struct tm *t)
{
   inline gfx_pos_t ax (gfx_pos_t a, gfx_pos_t l)
   {
      return 100 + l * ((int) gfx_cos[(a + 192) & 255] - 128) / 127;
   }
   inline gfx_pos_t ay (gfx_pos_t a, gfx_pos_t l)
   {
      return 100 - l * ((int) gfx_cos[(a) & 255] - 128) / 127;
   }
   for (int a = 0; a < 256; a += 4)
      gfx_line (ax (a, 99), ay (a, 99), ax (a + 4, 99), ay (a + 4, 99), 255);
   for (int h = 0; h < 12; h++)
      gfx_line (ax (h * 256 / 12, 99), ay (h * 256 / 12, 99), ax (h * 256 / 12, (h % 3) ? 90 : 80),
                ay (h * 256 / 12, (h % 3) ? 90 : 80), 255);
   gfx_line (100, 100, ax (t->tm_min * 256 / 60, 95), ay (t->tm_min * 256 / 60, 95), 255);
   int h = ((int) t->tm_hour * 60 + t->tm_min) * 256 / 12 / 60;
   for (int dx = -1; dx < 1; dx++)
      for (int dy = -1; dy < 1; dy++)
         gfx_line (100 + dx, 100 + dy, ax (h, 60) + dx, ay (h, 60) + dy, 255);
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
