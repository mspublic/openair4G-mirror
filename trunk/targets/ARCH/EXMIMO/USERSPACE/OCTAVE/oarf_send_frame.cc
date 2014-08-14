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
 
// Matthias Ihmig, 2013
// Maxime Guillaud - created Fri May 12 16:20:04 CEST 2006
// see http://www.gnu.org/software/octave/doc/interpreter/Dynamically-Linked-Functions.html#Dynamically-Linked-Functions
// and http://wiki.octave.org/wiki.pl?CodaTutorial
// and http://octave.sourceforge.net/coda/c58.html
// compilation: see Makefile
// Update: Wed May 23 17:25:39 CEST 2007, fifo acquisition of signal buffer (RK)


#include <octave/oct.h>

extern "C" {
#include "openair0_lib.h"
}

#define FCNNAME "oarf_send_frame"

#define TRACE 1

static bool any_bad_argument(const octave_value_list &args)
{
    octave_value v;
    if (args.length()!=3)
    {
        error(FCNNAME);
        error("syntax: oarf_send_frame(card,sig,nbits)\n      card (starting from 0), sig is a 2D array (size depends on no. of antennas and resampling factor), nbits is number of bits to quantize the signal to.");
        return true;
    }

    v=args(0);
    if ((!v.is_real_scalar()) || (v.scalar_value() < 0.0) || (floor(v.scalar_value()) != v.scalar_value()) || (v.scalar_value() >= MAX_CARDS))
    {
        error(FCNNAME);
        error("card must be between 0 and MAX_CARDS.");
        return true;
    }

    v=args(1);
    printf("signal: R %d, C %d\n",v.rows(),v.columns());

    v=args(2);
    if ((!v.is_real_scalar()) && (v.scalar_value()!=8) && (v.scalar_value()!=16))
    {
        error(FCNNAME);
        error("nbits must be either 8 (CBMIMO) or 16 (ExpressMIMO)bits.");
        return true;
    }

    return false;
}


DEFUN_DLD (oarf_send_frame, args, nargout,"Send frame")
{
    if (any_bad_argument(args))
        return octave_value_list();
           
    const int card = args(0).int_value();  
    ComplexMatrix dx = args(1).complex_matrix_value();

    octave_value returnvalue;
    int i, ret;
    unsigned int length,aa,nbits,numcols;
    unsigned int resampling_factor[4];

    ret = openair0_open();
    if ( ret != 0 )
    {
        error(FCNNAME);
        if (ret == -1)
            error("Error opening /dev/openair0");
        if (ret == -2)
            error("Error mapping bigshm");
        if (ret == -3)
            error("Error mapping RX or TX buffer");
        return octave_value(ret);
    }
    
    if (card <0 || card >= openair0_num_detected_cards) {
        error("Invalid card number!");
        return octave_value(-4);
    }
    
    numcols = args(1).columns();
    
    //printf("colums = %d, rows = %d\n\n\n", numcols, args(1).rows());

    if ( numcols<1 || (numcols > openair0_num_antennas[card]))
    {
        error(FCNNAME);
        error("input array must be of column size %d.", openair0_num_antennas[card]);
        return octave_value_list();
    }
    
    for (i=0;i<4;i++) {
      resampling_factor[i] = (openair0_exmimo_pci[card].exmimo_config_ptr)->framing.resampling_factor[i];
      printf("card %d, ant %d, resampling %d\n",card,i,resampling_factor[i]);
    }
    
    for (i=0;i<numcols;i++){
    if (args(1).rows()<(76800*(1 << (2-resampling_factor[i]))))
    {
        error(FCNNAME);
        error("input array column number %d must be of size %d.",i,(76800*(1 << (2-resampling_factor[i]))));
        return octave_value_list();
    }  
    }

    if ((openair0_exmimo_pci[card].exmimo_config_ptr->framing.tdd_config & TXRXSWITCH_MASK) != TXRXSWITCH_TESTTX)
      printf("Warning: tdd_config is not set to TXRXSWITCH_TESTTX! You better know what you are doing! :)\n");


    nbits = args(2).scalar_value();

    for (aa=0;aa<numcols;aa++)
    {
        if (nbits==16)
        {
            for (i=0;i<(76800*(1 << (2-resampling_factor[aa])));i++)
            {
	      //if (i<64) printf("%d: %d,%d\n",i,(short)real(dx(i,aa)),(short)imag(dx(i,aa)));
                ((short*) openair0_exmimo_pci[card].dac_head[aa])[2*i]     = (short)(real(dx(i,aa))); 
                ((short*) openair0_exmimo_pci[card].dac_head[aa])[1+(2*i)] = (short)(imag(dx(i,aa)));
            }
        }
        else if (nbits==8)
        {
            for (i=0;i<(76800*(1 << (2-resampling_factor[aa])));i++)
            {
	      //if (i<64) printf("%d: %d,%d\n",i,char(real(dx(i,aa))),char(imag(dx(i,aa))));
                ((char*) openair0_exmimo_pci[card].dac_head[aa])[2*i]     = char(real(dx(i,aa))); 
                ((char*) openair0_exmimo_pci[card].dac_head[aa])[1+(2*i)] = char(imag(dx(i,aa)));
            }
        }
        else {
            error(FCNNAME);
            error("nbits has to be 8 or 16!");
        }
    }

    openair0_start_rt_acquisition( card );
  
    openair0_close();

    return octave_value (dx);
}

