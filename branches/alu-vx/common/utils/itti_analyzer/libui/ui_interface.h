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

#include <glib.h>

#ifndef UI_INTERFACE_H_
#define UI_INTERFACE_H_

/*******************************************************************************
 * Functions used between dissectors and GUI to update signal dissection
 ******************************************************************************/

typedef gboolean (*ui_set_signal_text_cb_t) (gpointer user_data, gchar *text, gint length);

/*******************************************************************************
 * Pipe interface between GUI thread and other thread
 ******************************************************************************/

typedef gboolean (*pipe_input_cb_t) (gint source, gpointer user_data);

typedef struct {
    int             source_fd;
    guint           pipe_input_id;
    GIOChannel     *pipe_channel;

    pipe_input_cb_t input_cb;
    gpointer        user_data;
} pipe_input_t;

int ui_pipe_new(int pipe_fd[2], pipe_input_cb_t input_cb, gpointer user_data);

int ui_pipe_write_message(int pipe_fd, const uint16_t message_type,
                          const void * const message, const uint16_t message_size);

typedef struct {
    uint16_t message_size;
    uint16_t message_type;
} pipe_input_header_t;

enum ui_pipe_messages_id_e {
    /* Other thread -> GUI interface ids */
    UI_PIPE_CONNECTION_FAILED,
    UI_PIPE_CONNECTION_LOST,
    UI_PIPE_XML_DEFINITION,
    UI_PIPE_UPDATE_SIGNAL_LIST,

    /* GUI -> other threads */
    UI_PIPE_DISCONNECT_EVT
};

typedef struct {
    char  *xml_definition;
    size_t xml_definition_length;
} pipe_xml_definition_message_t;

typedef struct {
    GList *signal_list;
} pipe_new_signals_list_message_t;

#endif /* UI_INTERFACE_H_ */
