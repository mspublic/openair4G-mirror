#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define frand() ((int) rand() / (RAND_MAX+1))
#define eps 0.0000001 

inline int randomGen(int a, int b){

	// On suppose a<b
	
    return rand()%(b-a) +a;
}



typedef struct _list
{
    int val;
    struct _list *next;
} LIST;	

/* tri rapide sur une liste chainée  */
LIST* _quick_sort (LIST* list, LIST* end)
{
    LIST *pivot, *tmp, *next, *prec, *suiv;
    if ( list != end && list->next != end )
    {
        /* partitionnement (fin liste 'prec' : 'pivot'; fin liste 'suiv' : 'end') */
        pivot = list;
	prec = pivot;
	suiv = end;
        for ( tmp=list->next; tmp != end; tmp=next )
        {
            next = tmp->next;
            if (tmp->val > pivot->val)
                tmp->next = suiv, suiv = tmp; /* ajoute la cellule a la liste 'suiv' */
            else
                tmp->next = prec, prec = tmp; /* ajoute la cellule a la liste 'prec' */
        }
        /* appels recursifs */
        prec = _quick_sort (prec, pivot);
        suiv = _quick_sort (suiv, end);
        /* recolle la liste */
        pivot->next = suiv;
        return prec;
    }
    return list;
}

LIST* quick_sort (LIST* list)
{
    return _quick_sort (list, NULL);
}
void afficherListe(LIST* list)
{
    LIST* tmp = list;
    /* Tant que l'on n'est pas au bout de la liste */
    while(tmp != NULL)
    {
        /* On affiche */
        printf("%d ", tmp->val);
        /* On avance d'une case */
        tmp = tmp->next;
    }
}

LIST* ajouterEnFin(LIST* list, int valeur)
{
    /* On crée un nouvel élément */
    LIST* nouvelElement = malloc(sizeof(LIST));
 
    /* On assigne la valeur au nouvel élément */
    nouvelElement->val = valeur;
 
    /* On ajoute en fin, donc aucun élément ne va suivre */
    nouvelElement->next = NULL;
 
    if(list == NULL)
    {
        /* Si la liste est videé il suffit de renvoyer l'élément créé */
        return nouvelElement;
    }
    else
    {
        /* Sinon, on parcourt la liste à l'aide d'un pointeur temporaire et on
        indique que le dernier élément de la liste est relié au nouvel élément */
        LIST* temp=list;
        while(temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = nouvelElement;
        return list;
    }
}


int main(int argc, char **argv)
{
    LIST* list = NULL;
    LIST* list2 = NULL;
    int i;
 
    for(i=1;i<=10;i++)
    {
        //list = ajouterEnTete(list, i);
        list = ajouterEnFin(list, 11 - i);
    }
     list = ajouterEnFin(list, 20);
     list = ajouterEnFin(list, 77);
     list = ajouterEnFin(list, 0);
    //afficherListe(list);
        printf("\nsorting\n ");
    list2= quick_sort (list);
    //afficherListe(list2);
  
  int x =0;
   x = (int)sqrt(11);
	     //  printf("\nx %d \n ", x);

	/*int X_from = randomGen(0,100);
    	printf("\nX_from %d", X_from); 
	int Y_from = randomGen(0,150);
    	printf("\nY_from %d", Y_from); 
	int X_next = randomGen(0,100);
	 int X_to = X_next;
	int Y_next = randomGen(0, 150);
	int Y_to = Y_next;
    	printf("\ndestination: (%d, %d)", X_to, Y_to);

	int speed_next = randomGen(1, 150);
	int speed = speed_next;
    	printf("\nspeed_next %d", speed_next); //m/s
	float r = sqrt(pow(X_from - X_next, 2) + pow(Y_from - Y_next, 2));
	int distance = (int) r;  
	printf("\nr %f", r); 
	printf("\ndistance %d", distance); 
	float f =(distance/speed_next); 
	int journeyTime_next = (int) f;  //duration to get to dest
	////node->mob->journey_time = journeyTime_next;
	
    	printf("\n mob->journey_time_next %d",journeyTime_next );    	printf("\n mob->journey_time_next %f",f );*/


	/*float l =1.2547896658; //ma variable à tronquer
	int j= l*100; // autant de zéro de que nombre après la virgule voulu
	int k=j/100; //remettre le même nombre de zéro que précédemment 
	//printf("\nx j %d\n k %d\n ", j, k);
       float n = (int) (l*100 + .5) / 100;
	printf("\nx %f\n ",n);*/

	double b = 0.76779;
	float c = b*1000;
		float d = 112.3;
		int e = (int)(c);
		float f= (float)e/1000;
		//printf("\nb %f\nc %f\ne %d\nf %f\n ",b,c,e,f);
	double h = (double) ((int) (b*1000))/ 1000;
	double g = (double) ((int) (2.123456*1000))/ 1000;
		printf("\n h %.3f\n ",h);
		printf("\n g %.3f\n ",g);
	/*double s=0.000005;
	s = s*2;
	s = s/2;
	double z = 0.000005; 
	if (s <= z+eps ) {printf("\ns <= z+eps");}
	if ( s>= z-eps){printf("\ns>= z-eps");}
	if (s > z+eps ) {printf("\ns > z+eps");}
	if ( s< z-eps){printf("\ns< z-eps");}
	if ( (double)s >= z - eps &&  (double)s <= z + eps){printf("\ns == z");}
	{printf("\n");}*/
    return 0;
}



