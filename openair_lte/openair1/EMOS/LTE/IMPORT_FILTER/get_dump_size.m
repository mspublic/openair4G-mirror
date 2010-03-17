system('gcc dump_size.c -I../../..');
[status,result] = system('./a.out');
eval(result);