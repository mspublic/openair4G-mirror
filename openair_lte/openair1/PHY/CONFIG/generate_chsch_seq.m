% Generate pseudo-random QPSK sequence for CHSCH
% fk 20080801 changed the code to only produce positive values

fid=fopen('chsch.txt','w');
for i=0:31, 
  fprintf(fid,"chsch_seq_re[%d]: %x\nchsch_seq_im[%d]: %x\n",...
	  i,(2^16)*floor((2^16)*rand(1,1)) + floor((2^16)*rand(1,1)),...
	  i,(2^16)*floor((2^16)*rand(1,1)) + floor((2^16)*rand(1,1))); 
end ; 
fclose(fid)
