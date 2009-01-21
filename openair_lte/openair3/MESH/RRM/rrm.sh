
#!/bin/bash
killall emul_int
killall rrm

xterm -T CMM -hold -e ./emul_int  &
./rrm -i 2  2> log.txt
#sleep 1
#./emul_int

