// Menus

#include "revk.h"
#include "watchy.h"
#include "gfx.h"
#include "icons.h"

RTC_NOINIT_ATTR uint8_t btnlast;

void gfx_menu(void)
{ // Start menu
}

void
menu_show (struct tm *t)
{
	uint8_t btn=bits.buttons&~btnlast; // Pushed
	btnlast=bits.buttons;
	if(btn&&!menu1)
	{ // Any button starting
		menu1=1;menu2=0;menu3=0;
	}
	if(!menu2)
	{ // Showing top level menu
	}
}

