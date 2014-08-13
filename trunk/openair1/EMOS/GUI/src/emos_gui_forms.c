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
/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "emos_gui_forms.h"

FD_main_frm *create_form_main_frm(void)
{
  FL_OBJECT *obj;
  FD_main_frm *fdui = (FD_main_frm *) fl_calloc(1, sizeof(*fdui));

  fdui->main_frm = fl_bgn_form(FL_NO_BOX, 1010, 710);
  obj = fl_add_box(FL_FLAT_BOX,0,0,1010,710,"Main form");
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_MCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  obj = fl_add_box(FL_ROUNDED_BOX,10,610,230,90,"");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_COL1);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_box(FL_ROUNDED3D_DOWNBOX,5,5,850,600,"");
    fl_set_object_color(obj,FL_BLACK,FL_BLACK);
  fdui->sync_lbl = obj = fl_add_text(FL_NORMAL_TEXT,635,15,40,20,"sync");
    fl_set_object_boxtype(obj,FL_ROUNDED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->rec_lbl = obj = fl_add_text(FL_NORMAL_TEXT,720,15,40,20,"rec");
    fl_set_object_boxtype(obj,FL_ROUNDED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->pwr1_xyp = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,20,45,300,110,"Power");
    fl_set_object_color(obj,FL_BLACK,FL_RIGHT_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->pwr2_xyp = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,330,45,300,110,"Power");
    fl_set_object_color(obj,FL_BLACK,FL_RIGHT_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->rx2_lbl = obj = fl_add_text(FL_NORMAL_TEXT,345,10,80,40,"RX 2");
    fl_set_object_color(obj,FL_BLACK,FL_MCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->noise1_xyp = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,20,170,300,110,"N0/SNR");
    fl_set_object_color(obj,FL_BLACK,FL_RIGHT_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->noise2_xyp = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,330,170,300,110,"N0/SNR");
    fl_set_object_color(obj,FL_BLACK,FL_RIGHT_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->ch11_xyp = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,15,310,140,110,"CH TX1");
    fl_set_object_color(obj,FL_BLACK,FL_RIGHT_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->ch12_xyp = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,180,310,140,110,"CH TX2");
    fl_set_object_color(obj,FL_BLACK,FL_RIGHT_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->ch13_xyp = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,15,450,140,110,"CH TX3");
    fl_set_object_color(obj,FL_BLACK,FL_RIGHT_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->ch14_xyp = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,180,450,140,110,"CH TX4");
    fl_set_object_color(obj,FL_BLACK,FL_RIGHT_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->ch21_xyp = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,325,310,140,110,"CH TX1");
    fl_set_object_color(obj,FL_BLACK,FL_RIGHT_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->ch22_xyp = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,490,310,140,110,"CH TX2");
    fl_set_object_color(obj,FL_BLACK,FL_RIGHT_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->ch23_xyp = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,325,450,140,110,"CH TX3");
    fl_set_object_color(obj,FL_BLACK,FL_RIGHT_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->ch24_xyp = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,490,450,140,110,"CH TX4");
    fl_set_object_color(obj,FL_BLACK,FL_RIGHT_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->error_lbl = obj = fl_add_text(FL_NORMAL_TEXT,680,15,40,20,"error");
    fl_set_object_boxtype(obj,FL_ROUNDED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->gps_lbl = obj = fl_add_text(FL_NORMAL_TEXT,640,65,60,30,"gps");
    fl_set_object_boxtype(obj,FL_ROUNDED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->date_lbl = obj = fl_add_text(FL_NORMAL_TEXT,655,130,170,30,"date: ");
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->gps_lat_lbl = obj = fl_add_text(FL_NORMAL_TEXT,700,65,150,30,"lat: 0");
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->gps_lon_lbl = obj = fl_add_text(FL_NORMAL_TEXT,700,95,150,30,"lon: 0");
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->mode_lbl = obj = fl_add_text(FL_NORMAL_TEXT,640,160,90,20,"Mode: SGL");
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->idx_lbl = obj = fl_add_text(FL_NORMAL_TEXT,730,160,110,20,"Idx: ");
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_box(FL_ROUNDED_BOX,460,610,140,90,"");
    fl_set_object_color(obj,FL_DARKCYAN,FL_COL1);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->power_btn = obj = fl_add_button(FL_NORMAL_BUTTON,130,620,100,70,"PWR");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_callback(obj,power_callback,0);
  fdui->buffer_lbl = obj = fl_add_text(FL_NORMAL_TEXT,760,15,90,20,"Buffer: 100%");
    fl_set_object_boxtype(obj,FL_ROUNDED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->cap_xyp = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,640,310,200,250,"Capacity");
    fl_set_object_color(obj,FL_BLACK,FL_RIGHT_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->refresh_lbl = obj = fl_add_text(FL_NORMAL_TEXT,655,185,170,20,"Refresh interval: 1s");
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_box(FL_ROUNDED_BOX,860,90,140,220,"");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_COL1);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->refresh_dial = obj = fl_add_dial(FL_NORMAL_DIAL,890,110,80,70,"screen refresh rate");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_callback(obj,refresh_callback,0);
    fl_set_dial_bounds(obj, 0, 5);
    fl_set_dial_angles(obj, 45, 315);
    fl_set_dial_value(obj, 1);
    fl_set_dial_step(obj, 1);
    fl_set_dial_return(obj, FL_RETURN_CHANGED);
  obj = fl_add_box(FL_ROUNDED_BOX,860,460,140,140,"");
    fl_set_object_color(obj,FL_INDIANRED,FL_COL1);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->bler_lbl = obj = fl_add_text(FL_NORMAL_TEXT,650,210,180,20,"BLER:: 0");
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->n_recd_frames_lbl = obj = fl_add_text(FL_NORMAL_TEXT,650,235,180,20,"Rec'd frms: 0");
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->msg_text = obj = fl_add_text(FL_NORMAL_TEXT,25,570,810,30,"");
    fl_set_object_color(obj,FL_BLACK,FL_MCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON,860,5,140,70,"");
    fl_set_object_boxtype(obj,FL_ROUNDED_BOX);
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_pixmapbutton_file(obj, "Eurecom_Logo.xpm");
  fdui->rx1_lbl = obj = fl_add_text(FL_NORMAL_TEXT,35,10,80,40,"RX 1");
    fl_set_object_color(obj,FL_BLACK,FL_MCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_box(FL_ROUNDED_BOX,610,610,180,90,"");
    fl_set_object_color(obj,FL_WHEAT,FL_COL1);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->terminal_mode_btn = obj = fl_add_button(FL_NORMAL_BUTTON,635,620,130,20,"terminal mode");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,terminal_mode_callback,0);
  fdui->single_led = obj = fl_add_box(FL_EMBOSSED_BOX,640,650,10,10,"Single user");
    fl_set_object_color(obj,FL_RIGHT_BCOL,FL_COL1);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
  fdui->multi_led = obj = fl_add_box(FL_EMBOSSED_BOX,640,670,10,10,"Multi user");
    fl_set_object_color(obj,FL_RIGHT_BCOL,FL_COL1);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
  fdui->mmse_btn = obj = fl_add_button(FL_PUSH_BUTTON,480,655,100,20,"MMSE");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,rx_mode_button_callback,1);
  fdui->ml_btn = obj = fl_add_button(FL_PUSH_BUTTON,480,675,100,20,"ML");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,rx_mode_button_callback,2);
  fdui->cbmimo_lbl = obj = fl_add_text(FL_NORMAL_TEXT,665,40,70,20,"cbmimo");
    fl_set_object_boxtype(obj,FL_ROUNDED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->file_index_dial = obj = fl_add_dial(FL_NORMAL_DIAL,890,485,80,70,"file index");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_callback(obj,file_index_callback,0);
    fl_set_dial_bounds(obj, 0, 100);
    fl_set_dial_angles(obj, 10, 350);
    fl_set_dial_value(obj, 0);
    fl_set_dial_step(obj, 10);
  obj = fl_add_button(FL_NORMAL_BUTTON,870,560,40,30,"+");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,file_index_callback,1);
  obj = fl_add_button(FL_NORMAL_BUTTON,950,560,40,30,"-");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,file_index_callback,-1);
  obj = fl_add_box(FL_ROUNDED_BOX,860,320,140,130,"");
    fl_set_object_color(obj,FL_PALEGREEN,FL_COL1);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->num_sym_dial = obj = fl_add_dial(FL_NORMAL_DIAL,890,340,80,70,"number of symbols");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_dial_bounds(obj, 0, 5);
    fl_set_dial_angles(obj, 30, 360);
    fl_set_dial_value(obj, 0);
    fl_set_dial_step(obj, 0.5);
  fdui->fix_lbl = obj = fl_add_text(FL_NORMAL_TEXT,640,95,60,30,"NO");
    fl_set_object_boxtype(obj,FL_ROUNDED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_box(FL_ROUNDED_BOX,800,610,200,90,"");
    fl_set_object_color(obj,FL_INDIANRED,FL_COL1);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->rec_btn = obj = fl_add_button(FL_PUSH_BUTTON,820,620,160,70,"Record");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,record_callback,0);
  fdui->exit_btn = obj = fl_add_button(FL_NORMAL_BUTTON,20,660,90,30,"exit");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_TOMATO,FL_COL1);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,exit_callback,0);
  fdui->config_btn = obj = fl_add_button(FL_NORMAL_BUTTON,20,620,90,30,"config");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,config_btn_callback,0);
  obj = fl_add_box(FL_ROUNDED_BOX,250,610,200,90,"");
    fl_set_object_color(obj,FL_PALEGREEN,FL_COL1);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->terminal_btn1 = obj = fl_add_button(FL_PUSH_BUTTON,270,650,40,40,"CH1");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,terminal_button_callback,1);
  fdui->terminal_btn2 = obj = fl_add_button(FL_PUSH_BUTTON,310,650,40,40,"CH2");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,terminal_button_callback,2);
  fdui->terminal_btn3 = obj = fl_add_button(FL_PUSH_BUTTON,350,650,40,40,"MR1");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,terminal_button_callback,3);
  fdui->terminal_btn4 = obj = fl_add_button(FL_PUSH_BUTTON,390,650,40,40,"MR2");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,terminal_button_callback,4);
  obj = fl_add_text(FL_NORMAL_TEXT,300,620,110,20,"terminal number");
    fl_set_object_color(obj,FL_PALEGREEN,FL_MCOL);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_button(FL_NORMAL_BUTTON,870,180,40,30,"+");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,refresh_callback,1);
  obj = fl_add_button(FL_NORMAL_BUTTON,950,180,40,30,"-");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,refresh_callback,-1);
  fdui->time_domain_btn = obj = fl_add_button(FL_PUSH_BUTTON,870,235,60,20,"time");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,time_freq_callback,0);
  fdui->freq_domain_btn = obj = fl_add_button(FL_PUSH_BUTTON,930,235,60,20,"freq");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,time_freq_callback,1);
  obj = fl_add_text(FL_NORMAL_TEXT,875,215,110,20,"channel display");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_MCOL);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fdui->refresh_timer = obj = fl_add_timer(FL_HIDDEN_TIMER,810,720,10,10,"timer");
    fl_set_object_callback(obj,refresh_timer_callback,0);
  obj = fl_add_text(FL_NORMAL_TEXT,875,260,110,20,"N0/SNR");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_MCOL);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fdui->n0_btn = obj = fl_add_button(FL_PUSH_BUTTON,870,280,60,20,"N0");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,noise_snr_callback,0);
  fdui->snr_btn = obj = fl_add_button(FL_PUSH_BUTTON,930,280,60,20,"SNR");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,noise_snr_callback,1);
  fdui->num_sym_plus = obj = fl_add_button(FL_NORMAL_BUTTON,870,415,40,30,"+");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->num_sym_minus = obj = fl_add_button(FL_NORMAL_BUTTON,950,415,40,30,"-");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->rx_mode_lbl = obj = fl_add_text(FL_NORMAL_TEXT,650,260,180,20,"RX mode:");
    fl_set_object_color(obj,FL_BLACK,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->single_btn = obj = fl_add_button(FL_PUSH_BUTTON,480,635,100,20,"SINGLE");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,rx_mode_button_callback,0);
  obj = fl_add_text(FL_NORMAL_TEXT,480,615,100,20,"receiver mode");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fl_end_form();

  fdui->main_frm->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_splash_frm *create_form_splash_frm(void)
{
  FL_OBJECT *obj;
  FD_splash_frm *fdui = (FD_splash_frm *) fl_calloc(1, sizeof(*fdui));

  fdui->splash_frm = fl_bgn_form(FL_NO_BOX, 390, 280);
  obj = fl_add_box(FL_FLAT_BOX,0,0,390,280,"");
  obj = fl_add_pixmap(FL_NORMAL_PIXMAP,40,20,310,100,"");
  obj = fl_add_text(FL_NORMAL_TEXT,70,150,80,30,"text");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fl_end_form();

  fdui->splash_frm->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_config_dialog *create_form_config_dialog(void)
{
  FL_OBJECT *obj;
  FD_config_dialog *fdui = (FD_config_dialog *) fl_calloc(1, sizeof(*fdui));

  fdui->config_dialog = fl_bgn_form(FL_NO_BOX, 410, 120);
  obj = fl_add_box(FL_UP_BOX,0,0,410,120,"");
  fdui->cancel_btn = obj = fl_add_button(FL_NORMAL_BUTTON,80,75,100,30,"Cancel");
    fl_set_object_callback(obj,get_dir_callback,-1);
  fdui->ok_btn = obj = fl_add_button(FL_RETURN_BUTTON,225,75,100,30,"OK");
    fl_set_object_callback(obj,get_dir_callback,0);
  fdui->dir_input = obj = fl_add_input(FL_NORMAL_INPUT,80,30,300,30,"Directory");
  fl_end_form();

  fdui->config_dialog->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

