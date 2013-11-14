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

gboolean ui_callback_on_about(GtkWidget *widget,
                              gpointer   data);

void ui_signal_add_to_list(gpointer data,
                           gpointer user_data);

gboolean ui_callback_on_connect(GtkWidget *widget,
                                gpointer   data);

gboolean ui_callback_on_disconnect(GtkWidget *widget,
                                   gpointer   data);

gboolean ui_callback_on_tree_view_select(GtkWidget *widget,
                                         GdkEvent  *event,
                                         gpointer   data);

gboolean
ui_callback_on_select_signal(GtkTreeSelection *selection,
                             GtkTreeModel     *model,
                             GtkTreePath      *path,
                             gboolean          path_currently_selected,
                             gpointer          userdata);

gboolean ui_callback_signal_go_to(GtkWidget *widget,
                                  gpointer   data);

gboolean ui_callback_signal_go_to_first(GtkWidget *widget,
                                        gpointer   data);

gboolean ui_callback_signal_go_to_last(GtkWidget *widget,
                                       gpointer   data);

gboolean ui_callback_signal_clear_list(GtkWidget *widget,
                                       gpointer   data);

gboolean ui_pipe_callback(gint source, gpointer user_data);

gboolean ui_callback_on_menu_none(GtkWidget *widget,
                                  gpointer data);

gboolean ui_callback_on_menu_all(GtkWidget *widget,
                                 gpointer data);

gboolean ui_callback_on_menu_item_selected(GtkWidget *widget,
                                           gpointer data);

gboolean ui_callback_on_tree_column_header_click(GtkWidget *widget,
                                                 gpointer   data);
#endif /* UI_CALLBACKS_H_ */
