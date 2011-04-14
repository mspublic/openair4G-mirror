% Generate pseudo-random QPSK sequence for CHBCH

fid=fopen("chbch.txt","w");
for i=0:7, 
  fprintf(fid,"pilot_re[%d]: %x\npilot_im[%d]: %x\n",i,((2^16)*floor((2^16)*(rand(1,1)-.5))) + floor((2^16)*(rand(1,1)-.5)),i,((2^16)*floor((2^16)*(rand(1,1)-.5))) + floor((2^16)*(rand(1,1)-.5))); 
end ; 
fclose(fid)
