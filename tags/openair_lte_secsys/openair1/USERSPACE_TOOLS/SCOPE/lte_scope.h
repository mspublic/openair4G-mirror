/** Header file generated with fdesign on Wed Oct 28 16:17:23 2009.**/

#ifndef FD_lte_scope_h_
#define FD_lte_scope_h_

/** Callbacks, globals and object handlers **/


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *lte_scope;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *channel_t_re;
	FL_OBJECT *scatter_plot;
	FL_OBJECT *demod_out;
	FL_OBJECT *channel_f;
	FL_OBJECT *channel_t_im;
	FL_OBJECT *decoder_input;
	FL_OBJECT *scatter_plot2;
} FD_lte_scope;

extern FD_lte_scope * create_form_lte_scope(void);

#endif /* FD_lte_scope_h_ */
