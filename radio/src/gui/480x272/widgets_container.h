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

#ifndef _WIDGETS_CONTAINER_H_
#define _WIDGETS_CONTAINER_H_

#include <stdlib.h>
#include "widget.h"
#if defined(INTERACTIVE_WIDGETS)
#include "keys.h"
#include "interwidge.h"
#endif

class WidgetsContainerInterface
{
  public:
    virtual unsigned int getZonesCount() const = 0;

    virtual Zone getZone(unsigned int index) const = 0;

    inline Widget * getWidget(unsigned int index)
    {
      return widgets[index];
    }

    inline void setWidget(unsigned int index, Widget * widget)
    {
      widgets[index] = widget;
    }

    virtual void createWidget(unsigned int index, const WidgetFactory * factory) = 0;

  protected:
    Widget ** widgets;
};

template<int N, int O>
class WidgetsContainer: public WidgetsContainerInterface
{
  public:
    struct ZonePersistentData {
      char widgetName[10];
      Widget::PersistentData widgetData;
    };

    struct PersistentData {
      ZonePersistentData zones[N];
      ZoneOptionValue options[O];
    };

  public:
    WidgetsContainer(PersistentData * persistentData):
      persistentData(persistentData)
    {
      widgets = (Widget **)calloc(N, sizeof(Widget *));
#if defined(INTERACTIVE_WIDGETS)
      pages = -1;
#endif
    }

    virtual ~WidgetsContainer()
    {
      if (widgets) {
        for (uint8_t i=0; i<N; i++) {
          delete widgets[i];
        }
        free(widgets);
      }
    }

    virtual void createWidget(unsigned int index, const WidgetFactory * factory)
    {
#if defined(INTERACTIVE_WIDGETS)
      pages = -1;
#endif
      if (widgets) {
        memset(persistentData->zones[index].widgetName, 0, sizeof(persistentData->zones[index].widgetName));
        if (factory) {
          strncpy(persistentData->zones[index].widgetName, factory->getName(), sizeof(persistentData->zones[index].widgetName));
          widgets[index] = factory->create(getZone(index), &persistentData->zones[index].widgetData);
        }
        else {
          widgets[index] = NULL;
        }
      }
    }

    virtual void create()
    {
      memset(persistentData, 0, sizeof(PersistentData));
    }

    virtual void load()
    {
#if defined(INTERACTIVE_WIDGETS)
      pages = -1;
#endif
      if (widgets) {
        unsigned int count = getZonesCount();
        for (unsigned int i=0; i<count; i++) {
          delete widgets[i];
          if (persistentData->zones[i].widgetName[0]) {
            char name[sizeof(persistentData->zones[i].widgetName)+1];
            memset(name, 0, sizeof(name));
            strncpy(name, persistentData->zones[i].widgetName, sizeof(persistentData->zones[i].widgetName));
            widgets[i] = loadWidget(name, getZone(i), &persistentData->zones[i].widgetData);
          }
          else {
            widgets[i] = NULL;
          }
        }
      }
    }

#if defined(INTERACTIVE_WIDGETS)

    int getInteractiveIndex(int16_t page)
    {
      int index = 0;
      int loops = page + 1;
      for(int i = 0; i < loops; i++) {
        while(!(widgets[index]->getPages())) {
          index++;
          }
        index++;
       }
    index--;
    return(index);
    }

    virtual void refresh() { refresh(0, 0); }
    virtual void refresh(event_t event, int page)
    {
      int16_t pages = getPages();
      int16_t zc = getZonesCount();
      if (widgets) {
        for (int i = 0; i < zc; i++) {
          if (widgets[i]) {
            if (pages && (zc == 1)) { //full-screen interactive
              widgets[i]->refresh(event, page);
            }
            else if (pages && (zc > 1) && (i == getInteractiveIndex(page))) { // multi-zone interactive
              Zone zone = getZone(i);
              interWidge::drawActiveWidgetHighlight(zone);
              widgets[i]->refresh(event, 0);
            }
            else {
              widgets[i]->refresh(0, 0);
            }
          }
        }
      }
    }

protected:
    int pages;

public:
    virtual int getPages()
    {
      if (pages > -1) {
        return(pages);
      }
      pages = 0;
      if ( ! widgets) {
        return(pages);
      }
      int zc = getZonesCount();
      if (zc == 1) {
        if (widgets[0]) {
          pages = widgets[0]->getPages();
        }
        return(pages);
      }
      for (int i = 0; i < N; i++) {
        if (widgets[i]) {
          pages += widgets[i]->getPages();
        }
      }
      return(pages);
    }

#endif

    inline ZoneOptionValue * getOptionValue(unsigned int index) const
    {
      return &persistentData->options[index];
    }

    virtual unsigned int getZonesCount() const = 0;

    virtual Zone getZone(unsigned int index) const = 0;

#if !defined(INTERACTIVE_WIDGETS)
    virtual void refresh()
    {
      if (widgets) {
        for (int i=0; i<N; i++) {
          if (widgets[i]) {
            widgets[i]->refresh();
          }
        }
      }
    }
#endif

    virtual void background()
    {
      if (widgets) {
        for (int i=0; i<N; i++) {
          if (widgets[i]) {
            widgets[i]->background();
          }
        }
      }
    }

  protected:
    PersistentData * persistentData;
};

#endif // _WIDGETS_CONTAINER_H_
