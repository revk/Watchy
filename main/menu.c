// Menus

#include "revk.h"
#include "watchy.h"
#include "gfx.h"
#include "icons.h"

void menu_show (struct tm*t)
{
   if (bits.buttons == 9)
   {                            // TODO force upgrade
      bits.holdoff = 1;
      bits.wifi = 1;
      if (!revk_link_down () && !revk_shutting_down (NULL))
         revk_command ("upgrade", NULL);
   } else
      bits.holdoff = 0;
}
