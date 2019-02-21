/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
 
#if defined(INTERACTIVE_WIDGETS)
#include "opentx.h"

namespace interWidge {
  int8_t page = 0;
  int8_t pages = -1;
  int8_t lastDir = 1;
  bool lastEventWasExit = false;

  void init()
  {
    if (pages == -1) {
      pages = customScreens[g_model.view]->getPages();
      TRACE("Init set pages at %d", pages);
    }
  }

  int getMainViewsCount()  // this was local to view_main.cpp, and it's small.  Don't want to clutter other files more than I must.
  {
    for (int index=1; index<MAX_CUSTOM_SCREENS; index++) {
      if (!customScreens[index]) {
        return index;
      }
    }
    return MAX_CUSTOM_SCREENS;
  }

  void drawActiveWidgetHighlight(Zone zone)
  {
    LcdFlags color = TEXT_INVERTED_BGCOLOR; // pointless waste of RAM
    int padding = 4;
    int thick = 2;
    lcdDrawSolidRect(zone.x - padding, zone.y - padding, zone.w + 2 * padding, zone.h + 2 * padding, thick, color);
  }

  event_t filterEvents(event_t event)
  {
    if (event && (event == EVT_KEY_BREAK(KEY_EXIT))) { // filter to remove "spurious" EVT_KEY_BREAK(KEY_EXIT) events on return from system, model, and telemetry menus
      if(lastEventWasExit) {
        return(event);
      }
      else {
        lastEventWasExit = false;
        event = 0;
      }
    }
    if (event) {
      lastEventWasExit = (event == EVT_KEY_FIRST(KEY_EXIT)) ? true : false;
    }
    event_t key = event & ~_MSK_KEY_FLAGS;
//    event_t flags = event & _MSK_KEY_FLAGS;
    if ((key == KEY_PGUP) || (key == KEY_PGDN)) {  // don't pass PGUP or PGDN events
      event = 0;
    }
    return(event);  // This is simpler than expected. What's wrong?
  }

  bool userExitScreen(event_t event)
  {
    if (pages > 0) {
      killEvents(event);
      TRACE("userExitScreen refresh %d", page);
      customScreens[g_model.view]->refresh(EVT_KEY_LONG(KEY_EXIT), page);
      g_model.view = circularIncDec(g_model.view, lastDir, 0, getMainViewsCount()-1);
      pages = customScreens[g_model.view]->getPages(); 
      page = lastDir < 0 ? (pages ? pages - 1 : 0) : 0;
      return true;
    }
    else {
      return false;
    }
  }

  void userIncrementPage()
  {
      lastDir = 1;
      if (pages && page < (pages - 1)) {
        page++;
        TRACE("increment page: %d", page);
      }
      else {
        g_model.view = circularIncDec(g_model.view, +1, 0, getMainViewsCount()-1);
        pages = customScreens[g_model.view]->getPages();
        page = 0;
      }
  }

  void userDecrementPage()
  {
      lastDir = -1;      
      if (pages && page > 0) {
        page--;
      }
      else {
        g_model.view = circularIncDec(g_model.view, -1, 0, getMainViewsCount()-1);
        pages = customScreens[g_model.view]->getPages();
        page = pages ? pages - 1 : 0;
        TRACE("increment page: %d", page);
      }
  }

  bool screenIsInteractive()
  {
    return(pages > 0);
  }

  int getCurrentPage()
  {
    return(page);
  }

}
#if defined(INTERACTIVE_WIDGETS)
#else
#endif

#endif

