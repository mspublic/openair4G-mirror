OMV: openair mobility visualisor
To compile: 
1. install the required package: apt-get install qt-creator or go to http://qt.nokia.com/downloads to download the SDK 
   and install manually  
2. to generate the .pro used to setup the make files, run: qmake -project QT+=opengl
3. to generate the Makefile: qmake-qt4
4. to compile: make all

To Test in standalone mode :
./OMV (for the moment, it only displays the window and default values for the area dimensions and node number)
