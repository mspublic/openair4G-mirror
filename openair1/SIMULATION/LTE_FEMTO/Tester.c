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
	
	int n=5;
		
	char *pruebas[]={ "./femtosim -n1000 -s-5 -S20 -b-3 -a -I1 -w-3",
					  "./femtosim -n1000 -s-5 -S20 -b0 -a -I1 -w0",
					  "./femtosim -n1000 -s-5 -S20 -b1 -a -I1 -w1",
					  "./femtosim -n1000 -s-5 -S20 -b2 -a -I1 -w2",
					  "./femtosim -n1000 -s-5 -S20 -b3 -a -I1 -w3"
					  /*"./femtosim -n1000 -s-5 -S20 -b-203 -a -I2 -w-3",
					  "./femtosim -n1000 -s-5 -S20 -b200 -a -I2 -w0",
					  "./femtosim -n1000 -s-5 -S20 -b201 -a -I2 -w1",
					  "./femtosim -n1000 -s-5 -S20 -b202 -a -I2 -w2",
					  "./femtosim -n1000 -s-5 -S20 -b203 -a -I2 -w3"	*/
	};
					
	/*char *pruebas[]={ "./femtosim -n1000 -s-5 -S20 -b1 -a",
					  "./femtosim -n1000 -s-5 -S20 -b2 -a -I1 -w0",
					  "./femtosim -n1000 -s-5 -S20 -b3 -a -I1 -w-3",
					  "./femtosim -n1000 -s-5 -S20 -b4 -a -I1 -w3",
					  "./femtosim -n1000 -s-5 -S20 -b5 ",
					  "./femtosim -n1000 -s-5 -S20 -b6 -I1 -w0",
					  "./femtosim -n1000 -s-5 -S20 -b7 -I1 -w-3",
					  "./femtosim -n1000 -s-5 -S20 -b8 -I1 -w3",
	};
	* */

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
