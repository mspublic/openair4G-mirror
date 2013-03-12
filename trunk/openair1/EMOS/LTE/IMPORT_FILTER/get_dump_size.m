system('make dump_size');
[status,result] = system('./dump_size');
eval(result);
