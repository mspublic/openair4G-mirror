/*! \file pmip_dereg.c
* \brief MN Deregistration process 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/
#define PMIP
//#define _REENTRANT
//#define PMIP_PCAP_C
//#include "pmip_dereg.h"
/*#include "pmip_hnp_cache.h"
#    include <pcap.h>
#    include <stdlib.h>*/
#    include "netinet/in.h"
//#    include "debug.h"
//#    include <pthread.h>
# include "pmip_types.h"
# include "pmip_fsm.h"
#include "pmip_hnp_cache.h"
#include<stdio.h>
#include<string.h>

//#include <thread.h>

void *thr_ciscolog();

/*!
*  Construct message event and send it to the MAG FSM
* \param
*/
void pmip_dereg_msg_handler(struct in6_addr mn_iidP)
{
    msg_info_t message_dereg;
    memset(&message_dereg, 0, sizeof(msg_info_t));
    message_dereg.mn_iid = hw_address2eth_address(mn_iidP); //Car erreur Compil disant que type est incompat
    //message_dereg.mn_iid = mn_iidP; // Quand je ne passe pas par hw_address2eth_address
    message_dereg.msg_event = hasDEREG;
    mag_fsm(&message_dereg);
}

/*!
*  looping thread for capturing CISCO WLCCP paquets
* \param devname device name eth0 for example
* \param iif     interface
*/

void *thr_ciscolog(void *arg)
{
int i;

printf("[DEREG] - Thread DEREG starting...\n"); 

while (1) {

	int j=0;
	int espace=15;
        int b;
	//int convert;
	char addrmacByte[16];
        for (i=0;i<10000000;i++); {
			FILE *ptr_file;
			FILE *ptr_file2;
			int n=0;
			//int line=0;
                        int totalDisassociated=0;
			int addrmacdec;
	    		char buf[500];
			char addrmac[16];
			struct in6_addr mn_iid;	
			int compteur=0;		

	    		ptr_file =fopen("/var/log/cisco_ap.log","r");
	    		if (!ptr_file)
				return ;
	
// First reading of the file. Get the number of times that the word "Disassociated appeared"
				while ((fgets(buf,sizeof(buf), ptr_file)!=NULL)) {	
					//printf("%s",buf);
				
				//if (strstr(buf, "\n")) {
				//	line = line+1;		}	
					//intf("%d \n",line);					

						if (strstr(buf, "Disassociated")) {
						n = n+1;			
						totalDisassociated = n;							
						}
				}
	//printf("\nThere are %d lines in the log file \n",line);
			fclose(ptr_file);
			printf ("[DEREG] - At the first logfile reading, there were %d times the word Disassociated \n\n",totalDisassociated);
	
// Second Part
				ptr_file2 =fopen("/var/log/cisco_ap.log","r");
				if (!ptr_file2)
					return;
				n = 0;
				while (1) {
					//printf ("Juste avant la grande boucle \n");
					sleep(1); //To avoid to take the whole CPU resource

					while ((fgets(buf,sizeof(buf), ptr_file2)!=NULL)) {

						//printf ("Est rentre dans la grande boucle \n");
						//printf ("%s \n",buf);
						if (strstr(buf, "Disassociated"))
							n = n+1;
						//	printf ("%d \n",n);}					
	
						if (n > totalDisassociated) {
							printf ("[DEREG] - A MN has just left \n");
							while (j<sizeof(buf)) { 
								if (buf[j] == ' ') { espace--;}
								if (espace==0 && buf[j]!=' ' &&  buf[j] != '.') { addrmac[compteur++] = buf[j] ; } 
								//printf ("%d",j);
								j++;		} 
                                                        addrmac[compteur] = '\0';
							compteur=0;
							printf ("Avant conversion : %s \n", addrmac);

							for (b = 0 ; b < 6; b++) {

                                                           strncpy(addrmacByte, addrmac+2*b, 2); 
                                                           addrmacByte[2] = '\0';
                                                           sscanf (addrmacByte, "%02x", &mn_iid.s6_addr[10 + b]);
                                                        }
                                                        NIP6ADDR(&mn_iid);
							pmip_dereg_msg_handler(mn_iid);		
							printf ("[DEREG] The MAC address of the MN who just left is %s \n\n", addrmac);
							totalDisassociated = n;
							//printf ("\nAnd there were %d times the word Disassociated \n",totalDisassociated);
						} 
						} 					
					}
			printf ("N'est pas rentre dans la grande boucle \n");
			fclose(ptr_file2);
		} 
        }
}

/*
void pmip_dereg_loop()
{
    	int a=0;
	int b=1;
	int j=0;
	int espace=15;

		while ( b > a ){

			FILE *ptr_file;
			int n=0;
                        int totalDisassociated=0;
	    		char buf[100000];
			char addrmac[16] = "vide";
			struct in6_addr mn_iid;	
			int compteur=0;		

	    		ptr_file =fopen("/var/log/cisco_ap.log","r");
	    		if (!ptr_file)
				return;
	    		while (1) {
				if (fgets(buf,100000, ptr_file)!=NULL) {
					// printf("%s",buf);
					if (strstr(buf, "Disassociated")) {
					n = n+1;			
					totalDisassociated = n;
					
					while (j<sizeof(buf)) { 
						if (buf[j] == ' ') { espace--;}
						if (espace==0 && buf[j]!=' ') { addrmac[compteur] = buf[j] ;compteur++; } 
						//printf ("%d",j);
					j++;
							} 
					compteur=0;
					//printf ("%s \n", addrmac);
					mn_iid.s6_addr[15] = addrmac;
                                        //NIP6ADDR(addrmac6);	
					//printf ("%s \n", &mn_iid);
					pmip_dereg_msg_handler(mn_iid);		
					printf ("[DEREG] The MAC address of the MN who just left is %s \n", addrmac);
					} 
			} 					
		}
			a = b;
			fclose(ptr_file);
		//	sleep (1);		
		
		}
    }
*/
