
IF.MGMT has three build configurations which all ready to be used
through the Makefile. These three configurations are,

1) Debug Configuration

   Definition: Debug flags are on, optimisation is off, to be 
   run inside GDB, etc.

   Command: Use make target `debug_binary' as 
   $ make debug_binary
   and binary will be created in bin/

2) Dynamic Configuration

   Definition: Optimisation flags are on, dynamic linkage, for 
   deployment (along with necessary libraries) or local use

   Command: Use make target `dynamic_binary' as
   $ make dynamic_binary
   and binary will be created in bin/

3) Static Configuration

   Definition: Static linkage, to be sent to other people 
   and/or companies whom may not have necessary libraries 
   installed

   Command: Use make target `static_binary' as
   $ make static_binary
   and binary will be created in bin/

For Dynamic and Static configurations the make target `strip' may be used 
to save space by removing symbols in the binary as follows,

$ make strip

For Eclipse development see README.BUILD.ECLIPSE file.

Please let me know in case of a build error sending relevant library
versions, platform, etc. information.

- Baris Demiray <baris.demiray@eurecom.fr>
