
// Maxime Guillaud - created Fri May 12 16:20:04 CEST 2006
// Matthias Ihmig, 2013
// see http://www.gnu.org/software/octave/doc/interpreter/Dynamically-Linked-Functions.html#Dynamically-Linked-Functions
// and http://wiki.octave.org/wiki.pl?CodaTutorial
// and http://octave.sourceforge.net/coda/c58.html
// compilation: see Makefile
// Update: Wed May 23 17:25:39 CEST 2007, fifo acquisition of signal buffer (RK)
//         29.01.2013: adopted to new kernel driver


#include <octave/oct.h>

extern "C" {
#include "openair0_lib.h"
}

#define FCNNAME "oarf_get_frame"

#define TRACE 1

static bool any_bad_argument(const octave_value_list &args)
{
    octave_value v;
    if (args.length()!=1)
    {
        error(FCNNAME);
        error("syntax: oarf_get_frame(card)");
        return true;
    }

    v=args(0);
    if ((!v.is_real_scalar()) || (v.scalar_value() < 0) || (floor(v.scalar_value()) != v.scalar_value()) || (v.scalar_value() >= MAX_CARDS))
    {
        error(FCNNAME);
        error("card must be 0-3.\nTo get frame from all cards, set SYNCMODE_MASTER in framing.\nConfigure framing.sync_mode (master resp. slave) for each card.\n");
        return true;
    }
    return false;
}


DEFUN_DLD (oarf_get_frame, args, nargout,"Get frame (Action 5)")
{
    int numant;
    
    if (any_bad_argument(args))
        return octave_value_list();
       
    const int card = args(0).int_value();
    
    octave_value returnvalue;
    int i,aa;
    short *rx_sig[MAX_CARDS * MAX_ANTENNAS];
    int ret;

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
    
    if (card <-1 || card >= openair0_num_detected_cards)
        error("Invalid card number!");
    

    if (card == -1)
        numant = openair0_num_detected_cards * openair0_num_antennas[0];
    else
        numant = openair0_num_antennas[card];
    
    ComplexMatrix dx (FRAME_LENGTH_COMPLEX_SAMPLES, numant);

    // assign userspace pointers
    for (i=0; i<numant; i++)
    {
        rx_sig[i] = (short*) openair0_exmimo_pci[ i / (int)openair0_num_antennas[0] ].adc_head[i % openair0_num_antennas[0]];
        printf("adc_head[%i] = %p ", i, rx_sig[i]);
    }
    printf("\n");

    //  msg("Getting buffer...\n");
    openair0_get_frame(card);

    for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES; i++)
        for (aa=0; aa<numant; aa++)
            dx(i, aa) = Complex( rx_sig[aa][i*2], rx_sig[aa][i*2+1] );
    
    openair0_close();

    return octave_value (dx);
}


