
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
        error("syntax: oarf_send_frame(card,sig,nbits)\n      card (starting from 0), sig is a 2D or 4D vector (depending on no. of antennas), nbits is number of bits to quantize the signal to.");
        return true;
    }

    v=args(0);
    if ((!v.is_real_scalar()) || (v.scalar_value() < 0.0) || (floor(v.scalar_value()) != v.scalar_value()) || (v.scalar_value() >= MAX_CARDS))
    {
        error(FCNNAME);
        error("card must be 0, 1, 2, or 3.");
        return true;
    }

    v=args(1);
    printf("signal: R %d, C %d\n",v.rows(),v.columns());
    return false;

    v=args(2);
    if ((!v.is_real_scalar()) || (v.scalar_value()!=8) || (v.scalar_value()!=16))
    {
        error(FCNNAME);
        error("nbits must be either 8 (CBMIMO) or 16 (ExpressMIMO)bits.");
        return true;
    }
}


DEFUN_DLD (oarf_send_frame, args, nargout,"Send frame")
{
    if (any_bad_argument(args))
        return octave_value_list();
           
    const int card = args(0).int_value();  
    ComplexMatrix dx = args(1).complex_matrix_value();

    octave_value returnvalue;
    int i, ret;
    unsigned int length,aa,nbits;
    int dummy=0;

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
    
    if (card <0 || card >= openair0_num_detected_cards)
        error("Invalid card number!");
        
    printf("colums = %d, rows = %d\n\n\n", args(1).columns(), args(1).rows());

    if ((args(1).columns() != openair0_num_antennas[card]) || (args(1).rows()!=76800))
    {
        error(FCNNAME);
        error("input array must be of size (%d,%d)", 76800, openair0_num_antennas[card]);
        return octave_value_list();
    }

    nbits = args(2).scalar_value();

    for (aa=0;aa<openair0_num_antennas[card];aa++)
    {
        if (nbits==16)
        {
            for (i=0;i<76800;i++)
            {
                if (i<64)
                    printf("%d: %d,%d\n",i,(short)real(dx(i,aa)),(short)imag(dx(i,aa)));
                ((short*) openair0_exmimo_pci[card].dac_head[aa])[2*i]     = (short)(real(dx(i,aa))); 
                ((short*) openair0_exmimo_pci[card].dac_head[aa])[1+(2*i)] = (short)(imag(dx(i,aa)));
            }
        }
        else if (nbits==8)
        {
            for (i=0;i<76800;i++)
            {
                if (i<64)
                    printf("%d: %d,%d\n",i,char(real(dx(i,aa))),char(imag(dx(i,aa))));
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

