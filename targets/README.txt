-----------------------------------------
1) Folders and files description
-----------------------------------------

This directory contains different targets for OpenAirInterface.org implementations

CBMIMO1 - CardBus MIMO1 hardware target for x86/RTAI
EXMIMO  - ExpressMIMO Hardware target for x86/RTAI
SIMU    - Simulator/Emulator target for x86
DOCS    - Doxygen documentation generation for openair1/openair2

First, you must have all four openair SW directories, openair1, openair2, openair3, and targets at the same hierarchical level as this directory.

1. The following packages must be available on your system:

        - libblas and libblas-dev
        - libxml2 and libxml2-dev
        - libforms-bin libforms-dev
	- libatlas-base-dev and libatlas-headers
        - asn1c and the LTE ASN1 files (for more information on how to install this and generate the required files see the README file in openair2/RRC/LITE/MESSAGES) 

Makefile - This Makefile does basic checks of the code to verify that everything is properly installed, just run make to see what your options are.

----------------------------------------------------
2) Organization of the folders and their dependancies
----------------------------------------------------
The 3 folders have the following structures
   
    - EXAMPLES: there you can find some basic examples with/without the network interface
    - KERN: this folder includes the files for kernel space compilation targeting realtime operation (RF/emulation platform)
    - USER: this folder includes the files for user space compilation targeting soft realtime operation (emulation/simulation/debugging)

----------------------------------------------------------------
3)  How to use through a tutorial: run a simple experimentation
----------------------------------------------------------------
1. Up to know you should have creted a directory: mkdir openair4G

2. Check out the openair4G/trunk repository:  svn co https://svn.eurecom.fr/openairsvn/openair4G/trunk openair4G

3. Now, please check the following:
   
   - the env variables in .bashrc the following lines, and source them if necessaary :
    set OPENAIR_HOME variable and export the following:
    export OPENAIR1_DIR=$OPENAIR_HOME/openair4G/openair1
    export OPENAIR2_DIR=$OPENAIR_HOME/openair4G/openair2
    export OPENAIR3_DIR=$OPENAIR_HOME/openair4G/openair3
    export OPENAIR_TARGETS=$OPENAIR_HOME/openair4G/targets/
    
   - checkout and compile the asn1 messages 
     follow the README.txt in OPENAIR2_DIR/RRC/LITE/MESSAGES to checkou the ASN.1 RRC messages
     follow the OPENAIR2_DIR/RRC/LITE/MESSAGE/asn1c/asn1c/INSTALL to install the asn1c
     go back to $OPENAIR_TARGET and make install 
   - cd $OPENAIR_TARGET, and make a sanity check by doing make check   
   - Check out simple examples in $OPENAIR_TARGET/SIMU/EXAMPLES   
