
// Maxime Guillaud - created Wed May 10 19:23:30 CEST 2006
// see http://www.gnu.org/software/octave/doc/interpreter/Dynamically-Linked-Functions.html#Dynamically-Linked-Functions
// and http://octave.sourceforge.net/coda/c58.html
// compile with: mkoctfile -I/usr/local/include/gpib -lgpib gpib_send.cc

#include <octave/oct.h>
#include "ib.h"




static bool any_bad_argument(const octave_value_list &args)
{
  if (args.length()!=3)
  {
    error("gpib_send: syntax: gpib_send(gpib_board,gpib_device,gpib_str).");
    return true;
  }


  if (!args(0).is_real_scalar())
  {
      error("gpib_send: GPIB board number must be a positive integer.");
      return true;
  }
  if (args(0).scalar_value() < 0.0)
  {
      error("gpib_send: GPIB board number must be a positive integer.");
      return true;
  }
  if (floor(args(0).scalar_value()) != args(0).scalar_value())
  {
      error("gpib_send: GPIB board number must be a positive integer.");
      return true;
  }


  if (!args(1).is_real_scalar())
  {
      error("gpib_send: GPIB device number must be a positive integer.");
      return true;
  }

  if (args(1).scalar_value() < 0.0)
  {
      error("gpib_send: GPIB device number must be a positive integer.");
      return true;
  }

  if (floor(args(1).scalar_value()) != args(1).scalar_value())
  {
      error("gpib_send: GPIB device number must be a positive integer.");
      return true;
  }

  if (!args(2).is_string())
  {
      error("gpib_send: third parameter must be a char string.");
      return true;
  }

  return false;
}




DEFUN_DLD (gpib_send, args, nargout,"sends commands to the GPIB bus")
{
  if (any_bad_argument(args))
       return octave_value_list();


  const int gpib_board = args(0).int_value();
  const int gpib_device = args(1).int_value();
  const std::string gpib_string = args(2).string_value();

  unsigned short addlist[2] = {gpib_device, NOADDR};
    
  SendIFC(gpib_board);

  // ENABLE all on GPIB bus
  EnableRemote(gpib_board, addlist);
  // If error, Bye!
  if (ibsta & ERR)
  {
    error("gpib_send: Instrument enable failed!");
    return octave_value_list();
  }

  Send(gpib_board, gpib_device, gpib_string.c_str(), strlen(gpib_string.c_str()), NLend);
  if (ibsta & ERR)
  {
    error("gpib_send: Send failed!");
    return octave_value_list();
  }

  return octave_value_list();

}
