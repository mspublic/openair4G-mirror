
#!/bin/bash
killall emul_int
killall rrm


./rrm -i 2
sleep 1
./emul_int 
