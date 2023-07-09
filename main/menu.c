// Menus

#include "revk.h"
#include "watchy.h"
#include "gfx.h"
#include "icons.h"

uint32_t
menu_show (uint32_t menu, uint8_t buttons)
{
   if (buttons == 9 && !revk_link_down () && !revk_shutting_down (NULL))
      revk_command ("upgrade", NULL);
   return 0;
}
