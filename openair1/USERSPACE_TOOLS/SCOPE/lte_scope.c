/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "lte_scope.h"

FD_lte_scope *create_form_lte_scope(void)
{
  FL_OBJECT *obj;
  FD_lte_scope *fdui = (FD_lte_scope *) fl_calloc(1, sizeof(*fdui));

  fdui->lte_scope = fl_bgn_form(FL_NO_BOX, 780, 620);
  obj = fl_add_box(FL_ROUNDED_BOX,0,0,780,620,"");
    fl_set_object_color(obj,FL_BLACK,FL_BLUE);
  fdui->channel_t_re = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,20,20,280,100,"Time-Domain Channel (Real Component)");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_BLUE);
  fdui->scatter_plot = obj = fl_add_xyplot(FL_POINTS_XYPLOT,600,20,160,190,"Scatter Plot");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_GREEN);
  fdui->demod_out = obj = fl_add_xyplot(FL_POINTS_XYPLOT,20,420,570,180,"Demodulator Output");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_YELLOW);
  fdui->channel_f = obj = fl_add_xyplot(FL_IMPULSE_XYPLOT,20,140,570,90,"Frequency Bin Response");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_RED);
  fdui->channel_t_im = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,310,20,280,100,"Time-Domain Channel (Imaginaryl Component)");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_BLUE);
  fdui->decoder_input = obj = fl_add_xyplot(FL_POINTS_XYPLOT,20,240,570,170,"Decoder Input");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_CYAN);
  fdui->scatter_plot2 = obj = fl_add_xyplot(FL_POINTS_XYPLOT,600,410,160,190,"Scatter Plot");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_GREEN);
  fdui->scatter_plot1 = obj = fl_add_xyplot(FL_POINTS_XYPLOT,600,215,160,190,"Scatter Plot");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_GREEN);
  fl_end_form();

  fdui->lte_scope->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

