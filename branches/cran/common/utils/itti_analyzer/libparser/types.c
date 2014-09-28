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
#include <string.h>

#define G_LOG_DOMAIN ("PARSER")

#include "types.h"
#include "array_type.h"
#include "enum_type.h"
#include "fundamental_type.h"
#include "struct_type.h"
#include "union_type.h"
#include "typedef_type.h"
#include "enum_value_type.h"
#include "file_type.h"
#include "field_type.h"
#include "reference_type.h"
#include "pointer_type.h"

types_t *type_new(enum type_e type)
{
    types_t *new_p;

    new_p = calloc(1, sizeof(types_t));

    new_p->type = type;

    switch(type) {
        case TYPE_FUNCTION:
            /* Nothing to do for now. Display is done by pointer type */
            break;
        case TYPE_ENUMERATION:
            new_p->type_hr_display = enum_type_hr_display;
            new_p->type_file_print = enum_type_file_print;
            new_p->type_dissect_from_buffer = enum_type_dissect_from_buffer;
            break;
        case TYPE_FUNDAMENTAL:
            new_p->type_hr_display = fundamental_type_hr_display;
            new_p->type_file_print = fundamental_type_file_print;
            new_p->type_dissect_from_buffer = fundamental_dissect_from_buffer;
            break;
        case TYPE_STRUCT:
            new_p->type_hr_display = struct_type_hr_display;
            new_p->type_file_print = struct_type_file_print;
            new_p->type_dissect_from_buffer = struct_dissect_from_buffer;
            break;
        case TYPE_UNION:
            new_p->type_hr_display = union_type_hr_display;
            new_p->type_file_print = union_type_file_print;
            new_p->type_dissect_from_buffer = union_dissect_from_buffer;
            break;
        case TYPE_TYPEDEF:
            new_p->type_hr_display = typedef_type_hr_display;
            new_p->type_file_print = typedef_type_file_print;
            new_p->type_dissect_from_buffer = typedef_dissect_from_buffer;
            break;
        case TYPE_ENUMERATION_VALUE:
            new_p->type_hr_display = enum_value_type_hr_display;
            new_p->type_file_print = enum_value_file_print;
            new_p->type_dissect_from_buffer = enum_value_dissect_from_buffer;
            break;
        case TYPE_FILE:
            new_p->type_hr_display = file_type_hr_display;
            new_p->type_file_print = file_type_file_print;
//             new_p->type_dissect_from_buffer = file_dissect_from_buffer;
            break;
        case TYPE_FIELD:
            new_p->type_hr_display = field_type_hr_display;
            new_p->type_file_print = field_type_file_print;
            new_p->type_dissect_from_buffer = field_dissect_from_buffer;
            break;
        case TYPE_REFERENCE:
            new_p->type_hr_display = reference_type_hr_display;
            new_p->type_file_print = reference_type_file_print;
            new_p->type_dissect_from_buffer = reference_dissect_from_buffer;
            break;
        case TYPE_ARRAY:
            new_p->type_hr_display = array_type_hr_display;
            new_p->type_file_print = array_type_file_print;
            new_p->type_dissect_from_buffer = array_dissect_from_buffer;
            break;
        case TYPE_POINTER:
            new_p->type_hr_display = pointer_type_hr_display;
            new_p->type_file_print = pointer_type_file_print;
            new_p->type_dissect_from_buffer = pointer_dissect_from_buffer;
            break;
        default:
            break;
    }

    return new_p;
}

int types_insert_tail(types_t **head, types_t *to_insert)
{
    if (to_insert == NULL || head == NULL)
        return -1;
    if (*head == NULL) {
        *head = to_insert;
    } else {
        types_t *last = *head;
        while(last->next != NULL) {
            last = last->next;
        }
        last->next = to_insert;
        to_insert->previous = last;
    }
    to_insert->head = *head;
    return 0;
}

void types_hr_display(types_t *head)
{
    types_t *current = head;

    if (head == NULL) {
        printf("Empty list\n");
        /* Empty list */
        return;
    }

    while((current = current->next) != NULL)
    {
        current->type_hr_display(current, 0);
    }
}
