Status of openair2 RRC CELLULAR

updated March 18, 2009

1- Location of important files

   * top Makefile : openair2/Makefile
   * object code Makefile for user mode : SIMULATION/USER_TOOLS/LAYER2_SIM/Makefile
   * executable : SIMULATION/USER_TOOLS/LAYER2_SIM/mac_sim_rg and mac sim_mt
   * execution scripts : EXAMPLES/ETH_EMUL_CELLULAR
   * toplogy : SIMULATION/TOPOLOGIES

2- To compile

   > cd []/openair2
   > ./compile_all_mw  --> creates MT + RG then discards object files
   > ./make mac_sim_rg_cellular  --> creates RG
   > ./make mac_sim_mt_cellular  --> creates MT

   "make clean" must be done between MT and RG compilation to avoid sharing object files.

3- To execute

   > cd EXAMPLES/ETH_EMUL_CELLULAR
   > ./start_rg_user to start the RG
   > ./start_mt1_user to start the MT

   The executables must be started on different machines (eg. through an ssh terminal such as tenor)

4- External (to RRC/CELLULAR) files modified

   * Makefile
   * SIMULATION/USER_TOOLS/LAYER2_SIM/Makefile
   * EXAMPLES/ETH_EMUL_CELLULAR directory created
   * SIMULATION/USER_TOOLS/LAYER2_SIM/mac_sim.c (ifdef MESH)
   * LAYER2/MAC/vars.h + openair2_proc.c  (ifdef MESH)
   * RRC/L2_INTERFACE/openair_rrc_L2_interface.c
   * SIMULATION/PHY_EMULATION/TRANSPORT/multicast_link.c (customize MW testbed)

