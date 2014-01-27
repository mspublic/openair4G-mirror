#include <stdlib.h>
#include <time.h>
#include <stdio.h>
void main( int argc, char **argv){

	
	clock_t t_ini, t_fin;
	time_t tiempo = time(0);
	struct tm *tlocal ;
	char output[128];
 	int i,j,h,x,z;
 	
 	int interferencias[11];
 	
 	interferencias[0]=-15;
 	interferencias[1]=-5;
 	interferencias[2]=-3;
 	interferencias[3]=-2;
 	interferencias[4]=-1;
 	interferencias[5]=0;
 	interferencias[6]=1;
 	interferencias[7]=2;
 	interferencias[8]=3;
 	interferencias[9]=5;
 	interferencias[10]=15;
	
	FILE *output_fd ;
	output_fd= fopen("TesterControl.txt","w");
	double secs;
	
	
	char **pruebas;
	int n=(4*8*4*11)+1;
	
	pruebas= (char **) malloc(n*sizeof(char *));
	
	
	
	for(i=0;i<n; i++)
	{
		pruebas[i]=(char*)malloc(200*sizeof(char));
	}
	
	
	pruebas[0]= "./femtosim -n1000  -s0 -S25 -b0";
	
	i=1;
	x=1;
	for(j=0;j<8;j++)
	{
		for(h=0;h<4;h++) 
		{

		    for( z=0;z<11;z++)
		    {
				 //printf(" %d %d %d %d %d %d\n",interferencias[z],x,j,h,i,n);
		    
				sprintf(pruebas[i],"./femtosim -n1000  -s0 -S25 -a -I1 -w%d -b100%d -p%d,%d",interferencias[z],x,j,h);
				i++;				
				sprintf(pruebas[i],"./femtosim -n1000  -s0 -S25 -a -I1 -w%d -b200%d -p%d,%d -A1 -D",interferencias[z],x,j,h);
				i++;				
				sprintf(pruebas[i],"./femtosim -n1000  -s0 -S25 -I1 -w%d -b300%d -p%d,%d",interferencias[z],x,j,h);
				i++;				
				sprintf(pruebas[i],"./femtosim -n1000  -s0 -S25 -I1 -w%d -b400%d -p%d,%d -A1 -D",interferencias[z],x,j,h);
				i++;							
				x++;
				
				
				
		    }
		}
	}
	
						

	for(i=0;i<n;i++)
	{		
	 	  printf("\n%s",pruebas[i]);			
	 	  
		t_ini = clock();					
			fprintf(output_fd,"\n%s",pruebas[i]);			
	    	 tiempo = time(0);
			tlocal = localtime(&tiempo);
			strftime(output,128,"%d/%m/%y %H:%M:%S",tlocal);
			  
			fprintf(output_fd,"\n\tInicio: \t%s",output); 
			
			system(pruebas[i]);
				
       
			tiempo = time(0);
			tlocal = localtime(&tiempo);  
			strftime(output,128,"%d/%m/%y %H:%M:%S",tlocal);
			  
			fprintf(output_fd,"\n\tFin: \t%s",output); 
	
	}
	fclose(output_fd);
	
	


}