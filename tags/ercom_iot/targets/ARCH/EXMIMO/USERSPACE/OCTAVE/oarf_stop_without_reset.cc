// Riadh Ghaddab - created 2013-10-24
// see http://www.gnu.org/software/octave/doc/interpreter/Dynamically-Linked-Functions.html#Dynamically-Linked-Functions
// and http://wiki.octave.org/wiki.pl?CodaTutorial
// and http://octave.sourceforge.net/coda/c58.html
// compilation: see Makefile


#include <octave/oct.h>

extern "C" {
#include "openair0_lib.h"
}

#define FCNNAME "oarf_stop_without_reset"

#define TRACE 1


static bool any_bad_argument(const octave_value_list &args)
{
    octave_value v;
    if (args.length()!=1)
    {
        error(FCNNAME);
        error("syntax: oarf_stop(card)");
        return true;
    }

    v=args(0);
    if ((!v.is_real_scalar()) || (v.scalar_value() < -1) || (floor(v.scalar_value()) != v.scalar_value()) || (v.scalar_value() >= MAX_CARDS))
    {
        error(FCNNAME);
        error("card must be 0..number of cards-1.\nUse card = -1 to stop all cards.\n");
        return true;
    }

    return false;
}

DEFUN_DLD (oarf_stop, args, nargout,"Stop RT acquisition and write registers.")
{
    int a,b;
    if (any_bad_argument(args))
       return octave_value_list();

    int card = args(0).int_value();
    int ret;

    octave_value returnvalue;

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
        error("Invalid card number (num detected cards: %d, card: %d)!", openair0_num_detected_cards, card);

    if (card == -1) {
        for (card = 0; card < openair0_num_detected_cards; card++)
            returnvalue = openair0_stop( card );
    } else
        returnvalue = openair0_stop_without_reset( card );

    openair0_close( );

    return octave_value(returnvalue);
}

