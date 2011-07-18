function radio_channel_gen()

  x=[-120 -60 -70; -60 -120 -60;-70 -60 -120];
%x=[-120:-40]
x=-100
x=x./2+70
X=10.^(x./10)
wdfcwe


				%Frequecy response covariance matrix
N=40;
Nf=16;
alpha=2;
W=3.84;%Mhzh
tau=[0:N-1];%us
h=exp(-alpha *((tau+1)/10));
h=h/sum(h);
Tf=(exp(j*2*pi*tau));
H_f=zeros(1,Nf);
for k=0:Nf-1
H_f(k+1)=(Tf*exp(j*2*pi*k*W/Nf))*h';
end
E_Hf=H_f'*H_f;
K_h=sqrt(E_Hf)*2^(10)


		%16 bit fixed point conversion: 1 bit for sign, 7 bits for integer part, 8 bits for fractional part

F_r=zeros(Nf,Nf);
F_c=zeros(Nf,Nf);
for i=1:Nf
  for k=1:Nf
    F_r(i,k)=0x0000;
    r_part=real(K_h(i,k));
    if(r_part > 0)
      r_part_int=ceil(r_part);
    else
      r_part_int=ceil(r_part);
    end
    r_part_frac=abs(r_part-r_part_int);
    c_part=imag(K_h(i,k));
    if(c_part >= 0)
      c_part_int=ceil(c_part);
    else
      c_part_int=ceil(c_part);
    end
    c_part_frac=abs(c_part-c_part_int);
    if(abs(r_part_int) > 2^15)  
      %F_r(i,k)=0x7f00;
      F_r(i,k)=0x7fff;
    else
      F_r(i,k)=abs(r_part_int);%bitshift(abs(r_part_int),8) ;
    end
    if(r_part<0) 
      F_r(i,k)+=0x8000;
    end
   %
   % for m=1:8
   %   if(r_part_frac > 2^(-m))
%	F_r(i,k)+=2^(m-1);
%	r_part_frac-=2^(-m);
 %     end
 %   end

    if(abs(c_part_int) > 2^15)  
      F_c(i,k)=0x7fff;
    else
      F_c(i,k)=abs(c_part_int);%bitshift(abs(c_part_int),8) ;
    end
    if(c_part<0) 
      F_c(i,k)+=0x8000;
    end
   
    %for m=1:8
     % if(c_part_frac > 2^(-m))
	%F_c(i,k)+=2^(m-1);
	%c_part_frac-=2^(-m);
      %end
    %end

  end
end
KH=F_r+j*F_c
[myfile, msg] = fopen ('covariance_matrix.dat', 'w', 'vaxd');


for k=1:Nf
  for kk=1:Nf
    fprintf(myfile,"%d",real(KH(k,kk)));
    fprintf(myfile,"\t");
    fprintf(myfile,"%d",imag(KH(k,kk)));
    fprintf(myfile,"\n");
  end
end



fclose(myfile);
				%precision evaluation
%F_rec_r=zeros(Nf,Nf);
%F_rec_c=zeros(Nf,Nf);
%for i=1:Nf
%  for k=1:Nf
%    q=0;
%    p=F_r(i,k);
    %for m=0:7
    %  if(bitand(p,1)~=0);
%	q+=2^(-m-1);
 %     end
 %     p=bitshift(p,-1);
  %  end
%    sig=0;
%    if(bitand(p,0x0800)~=0)
%      p=p-0x0800;%(2^11);
%      sig=1;
%    end
%    q+=p;
%    if(sig > 0)
%      q=(-q);
%    end
%    F_rec_r(i,k)=q;

%    p=F_c(i,k);
%    q=0;
    %for m=0:7
    %  if(bitand(p,1)~=0);
%	q+=2^(-m-1);
 %     end
 %     p=bitshift(p,-1);
 %   end
%    sig=0;
%    if(bitand(p,0x0800)~=0)
%      p=p-0x0800;%(2^11);
%      sig=1;
%    end
%    q+=p;
%    if(sig > 0)
%      q=(-q);
%    end
%    F_rec_c(i,k)=q;
%  end
%end

%F_rec=F_rec_r+j*F_rec_c;
%K_h
%F_rec
%Err=(K_h-F_rec)./K_h
end