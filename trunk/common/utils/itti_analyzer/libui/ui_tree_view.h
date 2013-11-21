#ifndef UI_TREE_VIEW_H_
#define UI_TREE_VIEW_H_

#include "ui_filters.h"

typedef enum col_type_e
{
    COL_MSG_NUM = 0,
    COL_LTE_TIME,
    COL_MESSAGE,
    COL_FROM_TASK,
    COL_TO_TASK,
    COL_INSTANCE,

    COL_MESSAGE_ID,
    COL_FROM_TASK_ID,
    COL_TO_TASK_ID,
    COL_FOREGROUND,
    COL_BACKGROUND,

    COL_BUFFER,
    NUM_COLS
} col_type_t;

typedef enum ui_tree_view_menu_type_e
{
    MENU_MESSAGE = 0,
    MENU_FROM_TASK,
    MENU_TO_TASK,
    MENU_INSTANCE,
    NUM_MENU_TYPE,
} ui_tree_view_menu_type_t;

typedef struct ui_tree_view_menu_enable_s
{
    GtkWidget *menu_enable;
    ui_filter_item_t *filter_item;
} ui_tree_view_menu_enable_t;

extern GtkWidget *ui_tree_view_menu;
extern ui_tree_view_menu_enable_t ui_tree_view_menu_enable[NUM_MENU_TYPE];

extern GdkEventButton *ui_tree_view_last_event;

int ui_tree_view_create(GtkWidget *window, GtkWidget *vbox);

int ui_tree_view_new_signal_ind(const uint32_t message_number, const gchar *lte_time,
                                const uint32_t message_id, const char *message_name,
                                const uint32_t origin_task_id, const char *origin_task,
                                const uint32_t destination_task_id, const char *to_task,
                                uint32_t instance, gpointer buffer);

void ui_tree_view_destroy_list(GtkWidget *list);

void ui_tree_view_select_row(gint row);

void ui_tree_view_refilter(void);

guint ui_tree_view_get_filtered_number(void);

#endif /* UI_TREE_VIEW_H_ */
