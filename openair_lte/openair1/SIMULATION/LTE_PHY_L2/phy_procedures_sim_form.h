/** Header file generated with fdesign on Sat Jul 24 08:07:25 2010.**/

#ifndef FD_phy_procedures_sim_h_
#define FD_phy_procedures_sim_h_

/** Callbacks, globals and object handlers **/


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *phy_procedures_sim;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *pusch_constellation;
	FL_OBJECT *pdsch_constellation;
	FL_OBJECT *ch00;
} FD_phy_procedures_sim;

extern FD_phy_procedures_sim * create_form_phy_procedures_sim(void);

#endif /* FD_phy_procedures_sim_h_ */
