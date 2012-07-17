 
#include <forms.h>
#include<stdlib.h>
#include "MUMIMO_form.h"

 FD_MUMIMO *create_form_MUMIMO(int transmission_mode)
  {
    FL_OBJECT *obj;
    FD_MUMIMO *fdui = (FD_MUMIMO *) fl_calloc(1,sizeof(*fdui));
    
    fdui->MUMIMO = fl_bgn_form(FL_NO_BOX, 1160, 700);
    obj = fl_add_box(FL_EMBOSSED_BOX,0,0,1160,700,"");
    fl_set_object_color(obj,FL_GREY,FL_BLUE);
    //fl_set_object_color(obj,FL_YELLOW,FL_BLUE);
    fdui->subband = obj = fl_add_chart(FL_BAR_CHART,20,20,450,300,"Subband Allocation: 0 = No Transmission, 1 = SUMIMO, 2 = MUMIMO");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_RED);

 
    if (transmission_mode==5){
    fdui->piechart = obj = fl_add_chart(FL_PIE_CHART,50,360,250,275,"Distribution of Transmissions in Different Mode");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_RED);
    }
    fdui->plot_avg = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,620,20,520,300,"Average System Throughput");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_RED);


    fdui->plot_instant = obj = fl_add_xyplot(FL_IMPULSE_XYPLOT,620,360,520,300,"Instantaneous Throughput");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_ORANGE);
    //fl_set_object_color(obj,FL_BLACK,FL_YELLOW);

    if(transmission_mode == 5){
      obj = fl_add_box(FL_FLAT_BOX,330,360,130,30,"SUMIMO");
      fl_set_object_color(obj,FL_RED,FL_RED);
      obj = fl_add_box(FL_FLAT_BOX,330,400,130,30,"Partial MUMIMO");
      fl_set_object_color(obj,FL_GREEN,FL_GREEN);
      obj = fl_add_box(FL_FLAT_BOX,330,440,130,30,"FULL MUMIMO");
      fl_set_object_color(obj,FL_BLUE,FL_BLUE);
      obj = fl_add_box(FL_FLAT_BOX,330,480,130,30,"No Transmission");
      fl_set_object_color(obj,FL_WHITE,FL_WHITE);
    }
    else
      {
	obj = fl_add_box(FL_FLAT_BOX,200,400,130,30,"SUMIMO");
	fl_set_object_color(obj,FL_RED,FL_RED);
	obj = fl_add_box(FL_FLAT_BOX,200,440,130,30,"Partial MUMIMO");
	fl_set_object_color(obj,FL_GREEN,FL_GREEN);
	obj = fl_add_box(FL_FLAT_BOX,200,480,130,30,"FULL MUMIMO");
	fl_set_object_color(obj,FL_BLUE,FL_BLUE);
	obj = fl_add_box(FL_FLAT_BOX,200,520,130,30,"No Transmission");
	fl_set_object_color(obj,FL_WHITE,FL_WHITE);
	  }

    obj = fl_add_box(FL_FLAT_BOX,680,45,170,20,"Overall Average Throughput");
    fl_set_object_color(obj,FL_CYAN,FL_CYAN); 
    fl_set_object_lsize(obj,8);
    obj = fl_add_box(FL_FLAT_BOX,680,65,170,20,"Average Throughput over 10 Frames");
    fl_set_object_color(obj,FL_YELLOW,FL_YELLOW);
    fl_set_object_lsize(obj,8);

    obj = fl_add_box(FL_FLAT_BOX,470,20,20,20,"2");
    fl_set_object_color(obj,FL_GREY,FL_GREY); 
    //fl_set_object_color(obj,FL_YELLOW,FL_YELLOW);
    obj = fl_add_box(FL_FLAT_BOX,470,160,20,20,"1");
    fl_set_object_color(obj,FL_GREY,FL_GREY); 
    //fl_set_object_color(obj,FL_YELLOW,FL_YELLOW);
    obj = fl_add_box(FL_FLAT_BOX,470,300,20,20,"0");
    fl_set_object_color(obj,FL_GREY,FL_GREY); 
    //fl_set_object_color(obj,FL_YELLOW,FL_YELLOW);
    fl_end_form();
    
    fdui->MUMIMO->fdui = fdui;


    return fdui;
  }
