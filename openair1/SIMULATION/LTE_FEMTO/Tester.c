#include <stdlib.h>
#include <time.h>
#include <stdio.h>
void main( int argc, char **argv){

	
	clock_t t_ini, t_fin;
	time_t tiempo = time(0);
	struct tm *tlocal ;
	char output[128];
 	int i,j;
	
	FILE *output_fd ;
	output_fd= fopen("TesterControl.txt","w");
	double secs;
	
	int n=3;
		
	char *pruebas[]={ "./femtosim  -n1   -s-5 -S10 -b1",						  
					  "./femtosim  -n10  -s-5 -S10 -b2",						  
					  "./femtosim  -n100 -s-5 -S10 -b3",						  
	};
					
	

	for(i=0;i<n;i++)
	{		
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
