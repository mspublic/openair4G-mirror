/*******************************************************************************
    OpenAirInterface 
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is 
   included in this distribution in the file called "COPYING". If not, 
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr
  
  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/

#ifndef UI_FILTERS_H_
#define UI_FILTERS_H_

#include <stdint.h>

#include "itti_types.h"

#define SIGNAL_NAME_LENGTH  100
#define COLOR_SIZE          10

typedef enum
{
    FILTER_UNKNOWN, FILTER_MESSAGES, FILTER_ORIGIN_TASKS, FILTER_DESTINATION_TASKS, FILTER_INSTANCES,
} ui_filter_e;

typedef enum
{
    ENTRY_ENABLED_FALSE, ENTRY_ENABLED_TRUE, ENTRY_ENABLED_UNDEFINED,
} ui_entry_enabled_e;

typedef struct
{
    uint32_t id;
    char name[SIGNAL_NAME_LENGTH];
    uint8_t enabled;
    char foreground[COLOR_SIZE];
    char background[COLOR_SIZE];
    GtkWidget *menu_item;
} ui_filter_item_t;

typedef struct
{
    char *name;
    uint32_t allocated;
    uint32_t used;
    ui_filter_item_t *items;
} ui_filter_t;

typedef struct
{
    gboolean filters_enabled;
    ui_filter_t messages;
    ui_filter_t origin_tasks;
    ui_filter_t destination_tasks;
    ui_filter_t instances;
} ui_filters_t;

extern ui_filters_t ui_filters;

int ui_init_filters(int reset, int clear_ids);

gboolean ui_filters_enable(gboolean enabled);

int ui_filters_search_id(ui_filter_t *filter, uint32_t value);

void ui_filters_add(ui_filter_e filter, uint32_t value, const char *name, ui_entry_enabled_e entry_enabled,
                    const char *foreground, const char *background);

gboolean ui_filters_message_enabled(const uint32_t message, const uint32_t origin_task, const uint32_t destination_task,
                                    const uint32_t instance);

int ui_filters_read(const char *file_name);

int ui_filters_file_write(const char *file_name);

void ui_create_filter_menus(void);

void ui_destroy_filter_menus(void);

void ui_destroy_filter_menu(ui_filter_e filter);

void ui_show_filter_menu(GtkWidget **menu, ui_filter_t *filter);

#endif /* UI_FILTERS_H_ */
