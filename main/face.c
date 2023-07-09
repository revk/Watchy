// Watch faces

#include "face.h"
#include "iec18004.h"

const char *
gfx_qr (const char *value, gfx_pos_t posx, gfx_pos_t posy, uint8_t scale)
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
      int ox = (W - width * s) / 2;
      int oy = (H - width * s) / 2;
      if (scale)
      {
         ox = posx;
         oy = posy + 1 - width * scale;
         s = scale;
      }
      for (int y = 0; y < width; y++)
         for (int x = 0; x < width; x++)
            if (qr[width * y + x] & QR_TAG_BLACK)
               for (int dy = 0; dy < s; dy++)
                  for (int dx = 0; dx < s; dx++)
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
   gfx_qr ("HTTPS://WATCHY.REVK.UK", 0, 0, 0);
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
face_show (uint8_t face, time_t now)
{
   struct tm t;
   localtime_r (&now, &t);
   if (face >= sizeof (faces) / sizeof (*faces))
      face = 0;
   gfx_lock ();
   gfx_clear (0);
   faces[face] (&t);
   gfx_unlock ();
   gfx_wait ();

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
   strftime (temp, sizeof (temp), "%FT%H:%M%z", t);
   gfx_pos (199, 160, GFX_R | GFX_B);
   gfx_7seg (2, "%d", charging ? -battery : battery);
   gfx_qr (temp, 0, 199, 2);
   strftime (temp, sizeof (temp), "%a", t);
   gfx_pos (199, 199, GFX_R | GFX_B);
   gfx_text (-4, "%s", temp);
}
