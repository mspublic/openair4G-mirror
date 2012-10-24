#!/bin/sh
echo "----README------"
echo ""
echo "make sure that you have compiled the code with GPROF=1 and installed gprof and graphviz"

echo "set up params"

n_frames=10000
abstraction=1
ue="1 2 3"
ping="64 128 256 512 1024 1400"

if [ $abstraction = 1 ]; then 
    option=-a
else
    option=-A -s 10
fi;


echo "start oai profiling"
for i in $ue
do
  for j in $ping 
  do
  rm gmon.out gmon.txt
  echo "ping -s $j 10.0.1.2"
  ping -s $j 10.0.1.2 -i 0.5 > ping.a$abstraction.n$n_frames.ue$i.ping$j.log &
   echo "$OPENAIR_TARGETS/SIMU/USER/oaisim $option -n $n_frames -u $i > /dev/null"
$OPENAIR_TARGETS/SIMU/USER/oaisim $option -n $n_frames -u $i  > /dev/null 
  pkill ping
#mv $OPENAIR_TARGETS/SIMU/USER/gmon. .
  gprof $OPENAIR_TARGETS/SIMU/USER/oaisim > gmon.txt
  echo "$OPENAIR_TARGETS/SCRIPTS/gprof2dot.py gmon.txt > profoai.ue$ue.ping$ping.dot"
  $OPENAIR_TARGETS/SCRIPTS/PROFILING/gprof2dot.py gmon.txt > profoai.a$abstraction.n$n_frames.ue$i.ping$j.dot
  dot -Tpng profoai.a$abstraction.n$n_frames.ue$i.ping$j.dot > profoai.a$abstraction.n$n_frames.ue$i.ping$j.png
echo "oai profiling with $i ue is done"
done
done
echo "end oai profiling"
