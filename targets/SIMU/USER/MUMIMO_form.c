 
#include <forms.h>
#include<stdlib.h>
#include "MUMIMO_form.h"

 FD_MUMIMO *create_form_MUMIMO(int transmission_mode)
  {
    FL_OBJECT *obj;
    FD_MUMIMO *fdui = (FD_MUMIMO *) fl_calloc(1,sizeof(*fdui));
    
    fdui->MUMIMO = fl_bgn_form(FL_NO_BOX, 1280, 700);
    obj = fl_add_box(FL_EMBOSSED_BOX,0,0,1280,700,"");
    fl_set_object_color(obj,FL_GREY,FL_BLUE);
    fdui->subband = obj = fl_add_chart(FL_BAR_CHART,20,20,450,300,"Subband Allocation: 0 = No Allocation, 1 = SUMIMO, 2 = MUMIMO");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_RED);
    if (transmission_mode==5){
    fdui->piechart = obj = fl_add_chart(FL_PIE_CHART,50,360,250,275,"Distribution of Transmissions in Different Mode");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_RED);
    }
    fdui->plot_avg = obj = fl_add_xyplot(FL_POINTS_XYPLOT,620,20,520,300,"Average System Throughput");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_RED);

    fdui->plot_instant_SU = obj = fl_add_xyplot(FL_IMPULSE_XYPLOT,500,360,240,300,"SU-MIMO");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_RED);

    fdui->plot_instant_MU = obj = fl_add_xyplot(FL_IMPULSE_XYPLOT,760,360,240,300,"Partial MU-MIMO");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_GREEN);

    fdui->plot_instant_FMU = obj = fl_add_xyplot(FL_IMPULSE_XYPLOT,1020,360,240,300,"FULL MU-MIMO");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_BLUE);

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

    //fl_set_object_color(obj,FL_BLUE,FL_RED);
    //fl_add_text(FL_NORMAL_TEXT,15,20,15,20,"2");
    //fl_add_text(FL_NORMAL_TEXT,15,160,15,160,"1");
    //l_add_text(FL_NORMAL_TEXT,15,300,15,300,"0");
    //fl_set_object_color(obj,FL_BLACK,FL_WHITE);
    fl_end_form();
    
    fdui->MUMIMO->fdui = fdui;


    return fdui;
  }
