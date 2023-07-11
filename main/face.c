// Watch faces

#include "face.h"
#include "menu.h"
#include "iec18004.h"

const char *
gfx_qr (const char *value, uint8_t scale)
{                               // QR code
#ifndef CONFIG_GFX_NONE
   int W = gfx_width ();
   int H = gfx_height ();
   unsigned int width = 0;
 uint8_t *qr = qr_encode (strlen (value), value, widthp: &width, noquiet:scale ? 0 : 1);
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
   {
      const char *r;
      if (revk_shutting_down (&r))
      {
         gfx_pos (100, 130, GFX_C | GFX_B);
         gfx_text (-1, "%s", r);
      }
   }
   gfx_pos (0, 199, GFX_L | GFX_B);
   strftime (temp, sizeof (temp), "%FT%H:%M%z", t);
   gfx_qr (temp, 2);
   gfx_pos (199, 140, GFX_R | GFX_B | GFX_H);
   gfx_7seg (2, "%d", steps_read ());
   gfx_pos (199, 165, GFX_R | GFX_B | GFX_H);
   gfx_7seg (2, "%3d", battery);
   strftime (temp, sizeof (temp), "%a", t);
   if (bits.revkstarted)
   {
      gfx_icon2 (32, 32, bits.charging ? icon_power : NULL);
      gfx_icon2 (32, 32, !revk_link_down ()? icon_wifi : NULL);
      gfx_icon2 (32, 32, lwmqtt_connected (revk_mqtt (0)) ? icon_mqtt : NULL);
   }
   gfx_pos (199, 199, GFX_R | GFX_B | GFX_H);
   gfx_text (4, "%s", temp);
}

void
face_analogue (struct tm *t)
{
   gfx_pos (100, 100, GFX_C | GFX_M);
   gfx_text (5, "TODO");
}
