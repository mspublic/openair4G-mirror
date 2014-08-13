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
  
  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/
/** Header file generated with fdesign on Fri Aug 12 18:02:36 2011.**/

#ifndef FD_lte_scope_h_
#define FD_lte_scope_h_

/** Callbacks, globals and object handlers **/


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *lte_scope;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *fer;
	FL_OBJECT *channel_t_re;
	FL_OBJECT *demod_out;
	FL_OBJECT *channel_drs_time;
	FL_OBJECT *channel_t_im;
	FL_OBJECT *scatter_plot2;
	FL_OBJECT *channel_srs_time;
	FL_OBJECT *channel_srs;
	FL_OBJECT *channel_drs;
	FL_OBJECT *rssi;
} FD_lte_scope;

extern FD_lte_scope * create_form_lte_scope(void);

#endif /* FD_lte_scope_h_ */
