
#ifndef FD_MUMIMO_h_
#define FD_MUMIMO_h

typedef struct{
  FL_FORM *MUMIMO;
  void *vdata;
  char *cdata;
  long  ldata;
  FL_OBJECT *subband;
  FL_OBJECT *piechart;
  FL_OBJECT *plot_avg;
  FL_OBJECT *plot_instant;
  //FL_OBJECT *plot_instant_MU;
  //FL_OBJECT *plot_instant_FMU;
} FD_MUMIMO;


extern FD_MUMIMO * create_form_MUMIMO(int);

#endif
