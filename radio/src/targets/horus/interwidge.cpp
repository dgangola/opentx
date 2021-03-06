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
  enum specialCode_t : uint8_t { NONE, ENTER, EXIT };
  specialCode_t specialCode = NONE;
  bool lastEventWasExit = false;

  /*
  ** This gets called at the top of menuMainView. If pages is set to -1, it initializes some variables. This
  ** may be from the radio just powering up, or invalidate() being called. invalidate() is called when anything
  ** that can potentially change the main pages is called.
  */
  void init()
  {
    if (pages == -1) {
      pages = customScreens[g_model.view]->getPages();
      page = 0;
      // lastDir = 1; // we can probably leave the direction alone.  The user will find themselves at "page 0", but at least the direction will be the same
      specialCode = ENTER;  // signal start of interaction to whatever widget is showing, may be non-interactive, and ignore this
      lastEventWasExit = false;  // trap EVT_KEY_BREAK(KEY_EXIT) resulting from exiting other screens
    }
  }

  void invalidate()
  {
    customScreens[g_model.view]->refresh(EVT_KEY_LONG(KEY_EXIT), page);  // signal shutdown to widget
    pages = -1;  // trigger init on next menuMainView
  }

  int getMainViewsCount()  // this was local to view_main.cpp.  Don't want to clutter other files more than I must.
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
    int padding = 4;
    int thick = 2;
    lcdDrawSolidRect(zone.x - padding, zone.y - padding, zone.w + 2 * padding, zone.h + 2 * padding, thick, MAINVIEW_PANES_COLOR);
  }

  event_t filterEvents(event_t event)
  {
    if (event && (event == EVT_KEY_BREAK(KEY_EXIT))) { // filter to remove "spurious" EVT_KEY_BREAK(KEY_EXIT) events on return from system, model, and telemetry menus
      if(lastEventWasExit) { // rather than filtering, adding killEvents() after exiting these views might be better
        lastEventWasExit = false;
        return(event);
      }
      else {
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
    if ( ! event && specialCode != NONE) {
      if (specialCode == ENTER) {
        event = EVT_KEY_FIRST(KEY_PGDN);  // Question: Change for X12S, or keep it uniform?
      }
      else if (specialCode == EXIT) {
        event = EVT_KEY_LONG(KEY_EXIT);
      }
      specialCode = NONE;
    }
    return(event);
  }

  bool userExitScreen(event_t event)
  {
    if (pages > 0) {
      killEvents(event);
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


// the next two funcs should be rolled into one
  void userIncrementPage()
  {
    lastDir = 1;
    if (pages && page < (pages - 1)) {
      if (customScreens[g_model.view]->getZonesCount() != 1) { // multi-zone interactive
        customScreens[g_model.view]->refresh(EVT_KEY_LONG(KEY_EXIT), page);
        specialCode = ENTER;
      }
      page++;
    }
    else {
      if (pages) {
        customScreens[g_model.view]->refresh(EVT_KEY_LONG(KEY_EXIT), page); // either single or multi - we're leaving the page
      }
      g_model.view = circularIncDec(g_model.view, +1, 0, getMainViewsCount()-1);
      pages = customScreens[g_model.view]->getPages();
      page = 0;
      if (pages) {
        specialCode = ENTER;
      }
    }
  }

  void userDecrementPage()
  {
    lastDir = -1;
    if (pages && page > 0) {
      if (customScreens[g_model.view]->getZonesCount() != 1) { // multi-zone interactive
        customScreens[g_model.view]->refresh(EVT_KEY_LONG(KEY_EXIT), page);
        specialCode = ENTER;
      }
      page--;
    }
    else {
      if (pages) {
        customScreens[g_model.view]->refresh(EVT_KEY_LONG(KEY_EXIT), page); // either single or multi - we're leaving the page
      }
      g_model.view = circularIncDec(g_model.view, -1, 0, getMainViewsCount()-1);
      pages = customScreens[g_model.view]->getPages();
      page = pages ? pages - 1 : 0;
      if (pages) {
        specialCode = ENTER;
      }
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

#endif

