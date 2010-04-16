system('gcc dump_size.c -I../../.. -msse2');
[status,result] = system('./a.out');
eval(result);