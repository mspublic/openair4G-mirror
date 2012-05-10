system('gcc dump_size.c -I../../.. -DNO_RTAI -msse2');
[status,result] = system('./a.out');
eval(result);