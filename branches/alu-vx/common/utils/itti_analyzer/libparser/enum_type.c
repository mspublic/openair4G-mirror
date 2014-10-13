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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define G_LOG_DOMAIN ("PARSER")

#include "enum_type.h"
#include "ui_interface.h"

char *enum_type_get_name_from_value(types_t *type, uint32_t value)
{
    char    *enum_name = "UNKNOWN";
    types_t *enum_value;

    /* Loop on each enumeration values */
    for (enum_value = type->child; enum_value; enum_value = enum_value->next) {
        if (value == enum_value->init_value) {
            enum_name = enum_value->name;
            break;
        }
    }
    return enum_name;
}

int enum_type_dissect_from_buffer(
    types_t *type, ui_set_signal_text_cb_t ui_set_signal_text_cb, gpointer user_data,
    buffer_t *buffer, uint32_t offset, uint32_t parent_offset, int indent, gboolean new_line)
{
    uint32_t value = 0;
    types_t *values;

    DISPLAY_PARSE_INFO("enum", type->name, offset, parent_offset);

    value = buffer_get_uint32_t(buffer, parent_offset + offset);

    for (values = type->child; values; values = values->next) {
        if (value == values->init_value) {
            values->parent = type;
            values->type_dissect_from_buffer(
                values, ui_set_signal_text_cb, user_data, buffer, offset, parent_offset,
                type->name == NULL ? indent: indent + DISPLAY_TAB_SIZE, FALSE);
            break;
        }
    }
    if (values == NULL) {
        int length = 0;
        char cbuf[50];

        length = sprintf(cbuf, "(0x%08x) UNKNOWN;\n", value);
        ui_set_signal_text_cb(user_data, cbuf, length);
    }

    return 0;
}

int enum_type_file_print(types_t *type, int indent, FILE *file)
{
    types_t *values;

    if (type == NULL)
        return -1;
    INDENTED(file, indent,   fprintf(file, "<Enumeration>\n"));
    INDENTED(file, indent+4, fprintf(file, "Name .......: %s\n", type->name));
    INDENTED(file, indent+4, fprintf(file, "Size .......: %d\n", type->size));
    INDENTED(file, indent+4, fprintf(file, "Align ......: %d\n", type->align));
    INDENTED(file, indent+4, fprintf(file, "Artificial .: %d\n", type->artificial));
    INDENTED(file, indent+4, fprintf(file, "File .......: %s\n", type->file));
    INDENTED(file, indent+4, fprintf(file, "Line .......: %d\n", type->line));
    if (type->file_ref != NULL)
        type->file_ref->type_file_print(type->file_ref, indent+4, file);
    /* Call enum values display */
    for (values = type->child; values; values = values->next) {
        values->type_file_print(values, indent + 4, file);
    }
    INDENTED(file, indent, fprintf(file, "</Enumeration>\n"));

    return 0;
}

int enum_type_hr_display(types_t *type, int indent)
{
    types_t *values;

    if (type == NULL)
        return -1;
    INDENTED(stdout, indent,   printf("<Enumeration>\n"));
    INDENTED(stdout, indent+4, printf("Name .......: %s\n", type->name));
    INDENTED(stdout, indent+4, printf("Size .......: %d\n", type->size));
    INDENTED(stdout, indent+4, printf("Align ......: %d\n", type->align));
    INDENTED(stdout, indent+4, printf("Artificial .: %d\n", type->artificial));
    INDENTED(stdout, indent+4, printf("File .......: %s\n", type->file));
    INDENTED(stdout, indent+4, printf("Line .......: %d\n", type->line));
    if (type->file_ref != NULL)
        type->file_ref->type_hr_display(type->file_ref, indent+4);
    /* Call enum values display */
    for (values = type->child; values; values = values->next) {
        values->type_hr_display(values, indent + 4);
    }
    INDENTED(stdout, indent, printf("</Enumeration>\n"));

    return 0;
}
