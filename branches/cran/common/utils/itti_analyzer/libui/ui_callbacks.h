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

#ifndef UI_CALLBACKS_H_
#define UI_CALLBACKS_H_

#include <gtk/gtk.h>

gboolean ui_callback_on_open_messages(GtkWidget *widget,
                                      gpointer   data);

gboolean ui_callback_on_save_messages(GtkWidget *widget,
                                      gpointer   data);

gboolean ui_callback_on_filters_enabled(GtkToolButton *button,
                                        gpointer data);

gboolean ui_callback_on_open_filters(GtkWidget *widget,
                                     gpointer data);

gboolean ui_callback_on_save_filters(GtkWidget *widget,
                                     gpointer data);

gboolean ui_callback_on_enable_filters(GtkWidget *widget,
                                       gpointer data);

gboolean ui_callback_on_about(GtkWidget *widget,
                              gpointer   data);

gint ui_callback_check_string (const char *string,
                               const gint lenght,
                               const guint message_number);

gboolean ui_pipe_callback(gint source, gpointer user_data);

gboolean ui_callback_on_auto_reconnect(GtkWidget *widget,
                                       gpointer data);

void ui_callback_dialogbox_connect_destroy(void);

gboolean ui_callback_on_connect(GtkWidget *widget,
                                gpointer   data);

gboolean ui_callback_on_disconnect(GtkWidget *widget,
                                   gpointer   data);

gboolean ui_callback_on_tree_view_select(GtkWidget *widget,
                                         GdkEvent  *event,
                                         gpointer   data);

gboolean ui_callback_on_select_signal(GtkTreeSelection *selection,
                                      GtkTreeModel     *model,
                                      GtkTreePath      *path,
                                      gboolean          path_currently_selected,
                                      gpointer          userdata);

void ui_signal_add_to_list(gpointer data,
                           gpointer user_data);

gboolean ui_callback_on_menu_enable (GtkWidget *widget, gpointer data);

gboolean ui_callback_on_menu_color (GtkWidget *widget, gpointer data);

gboolean ui_callback_signal_go_to_first(GtkWidget *widget,
                                        gpointer   data);

gboolean ui_callback_signal_go_to(GtkWidget *widget,
                                  gpointer data);

gboolean ui_callback_signal_go_to_entry(GtkWidget *widget,
                                        gpointer   data);

gboolean ui_callback_signal_go_to_last(GtkWidget *widget,
                                       gpointer   data);

gboolean ui_callback_display_message_header(GtkWidget *widget,
                                            gpointer data);

gboolean ui_callback_display_brace(GtkWidget *widget,
                                   gpointer data);

gboolean ui_callback_signal_clear_list(GtkWidget *widget,
                                       gpointer   data);

gboolean ui_callback_on_menu_none(GtkWidget *widget,
                                  gpointer data);

gboolean ui_callback_on_menu_all(GtkWidget *widget,
                                 gpointer data);

gboolean ui_callback_on_menu_item_selected(GtkWidget *widget,
                                           gpointer data);

gboolean ui_callback_on_tree_column_header_click(GtkWidget *widget,
                                                 gpointer   data);
#endif /* UI_CALLBACKS_H_ */
