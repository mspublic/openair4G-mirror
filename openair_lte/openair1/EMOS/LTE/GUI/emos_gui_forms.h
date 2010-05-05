/** Header file generated with fdesign on Wed May  5 09:35:39 2010.**/

#ifndef FD_main_frm_h_
#define FD_main_frm_h_

/** Callbacks, globals and object handlers **/
extern void power_callback(FL_OBJECT *, long);
extern void refresh_callback(FL_OBJECT *, long);
extern void rx_mode_button_callback(FL_OBJECT *, long);
extern void file_index_callback(FL_OBJECT *, long);
extern void record_callback(FL_OBJECT *, long);
extern void exit_callback(FL_OBJECT *, long);
extern void config_btn_callback(FL_OBJECT *, long);
extern void time_freq_callback(FL_OBJECT *, long);
extern void refresh_timer_callback(FL_OBJECT *, long);
extern void noise_snr_callback(FL_OBJECT *, long);
extern void terminal_button_callback(FL_OBJECT *, long);
extern void checkpoint_callback(FL_OBJECT *, long);
extern void input_callback(FL_OBJECT *, long);


extern void get_dir_callback(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *main_frm;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *sync_lbl;
	FL_OBJECT *rec_lbl;
	FL_OBJECT *pwr1_xyp;
	FL_OBJECT *pwr2_xyp;
	FL_OBJECT *rx2_lbl;
	FL_OBJECT *noise1_xyp;
	FL_OBJECT *noise2_xyp;
	FL_OBJECT *ch11_sec0_xyp;
	FL_OBJECT *ch12_sec0_xyp;
	FL_OBJECT *ch21_sec0_xyp;
	FL_OBJECT *ch22_sec0_xyp;
	FL_OBJECT *ch11_sec1_xyp;
	FL_OBJECT *ch12_sec1_xyp;
	FL_OBJECT *ch21_sec1_xyp;
	FL_OBJECT *ch22_sec1_xyp;
	FL_OBJECT *error_lbl;
	FL_OBJECT *gps_lbl;
	FL_OBJECT *date_lbl;
	FL_OBJECT *gps_lat_lbl;
	FL_OBJECT *gps_lon_lbl;
	FL_OBJECT *idx_lbl;
	FL_OBJECT *power_btn;
	FL_OBJECT *buffer_lbl;
	FL_OBJECT *refresh_lbl;
	FL_OBJECT *refresh_dial;
	FL_OBJECT *bler_lbl;
	FL_OBJECT *n_recd_frames_lbl;
	FL_OBJECT *msg_text;
	FL_OBJECT *rx1_lbl;
	FL_OBJECT *alamouti_btn;
	FL_OBJECT *precoding_btn;
	FL_OBJECT *cbmimo_lbl;
	FL_OBJECT *file_index_dial;
	FL_OBJECT *fix_lbl;
	FL_OBJECT *rec_btn;
	FL_OBJECT *exit_btn;
	FL_OBJECT *config_btn;
	FL_OBJECT *time_domain_btn;
	FL_OBJECT *freq_domain_btn;
	FL_OBJECT *refresh_timer;
	FL_OBJECT *n0_btn;
	FL_OBJECT *snr_btn;
	FL_OBJECT *rx_mode_lbl;
	FL_OBJECT *siso_btn;
	FL_OBJECT *ch11_sec2_xyp;
	FL_OBJECT *ch12_sec2_xyp;
	FL_OBJECT *ch21_sec2_xyp;
	FL_OBJECT *ch22_sec2_xyp;
	FL_OBJECT *sec0_lbl;
	FL_OBJECT *sec1_lbl;
	FL_OBJECT *sec2_lbl;
	FL_OBJECT *terminal_btn1;
	FL_OBJECT *terminal_btn3;
	FL_OBJECT *next_cp;
	FL_OBJECT *rf_gain_txt;
	FL_OBJECT *digital_gain_txt;
	FL_OBJECT *rf_mode_txt;
} FD_main_frm;

extern FD_main_frm * create_form_main_frm(void);
typedef struct {
	FL_FORM *splash_frm;
	void *vdata;
	char *cdata;
	long  ldata;
} FD_splash_frm;

extern FD_splash_frm * create_form_splash_frm(void);
typedef struct {
	FL_FORM *config_dialog;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *cancel_btn;
	FL_OBJECT *ok_btn;
	FL_OBJECT *dir_input;
} FD_config_dialog;

extern FD_config_dialog * create_form_config_dialog(void);

#endif /* FD_main_frm_h_ */
