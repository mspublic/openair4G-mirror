#include"cor_SF_sim.h"
void choldc(double **ma, int nb);

void displayMat(double **m, int nbr1, int nbc1)
{
  int i,j;
   for(i=0;i<nbr1;i++)
    {
       for(j=0;j<nbr1;j++)
        {
          printf("%f\t",m[i][j]);
        }
	  putchar('\n');
    }
}
void choldc(double **ma, int nb)

{
 int i,j,k,l;
 double som;
 double *diag;
 diag = malloc(nb*sizeof(double));
 for(l=0;l<nb;l++)
  diag[l]=0;
 for(i=0;i<nb;i++)
  {
   for(j=i;j<nb;j++)
    {
      	
      //printf("%f\t %d \t %d \t %d \n",som,nb,j,i);
     for(som=ma[i][j],k=0;k<i-1;k++)
      {
       som-=ma[i][k]*ma[j][k];
      }
     if(j==i)
      {
	
       if (som<0)
        {
         printf("sum is equal to %f \t %d \t %d \t %d \n",som,nb,j,i);
         printf("matrice is not positive semi definite\n");
         exit(0);
        }
	ma[j][i]=sqrt(som);
	diag[i]=sqrt(som);
	printf(" %f \t %d \t %d \t %d \n",diag[i],nb,j,i);
      }else
        {
	 ma[j][i]=som/diag[i];
	 printf(" %f \t %d \t %d \t %d \n",ma[j][i],nb,j,i);
        }
    }
  }
  free(diag);
}
void cholDec (double **L, int nbr, int nbc)
{
  int alpha, beta, gamma, lambda;
  double sum = 0;
  double sum1 = 0;
  for (alpha = 0; alpha < nbr; alpha++)
    {
      printf("alpha=%d\n",alpha);
      for (beta = 0; beta < nbc; beta++)
	{
	  printf("beta=%d\n",beta);
	  if (beta == 0)
	    {
	      L[alpha][beta] = L[alpha][beta];
	    }
	  else if (beta == alpha)
	    {
	      for (gamma = 0; gamma < beta - 1; gamma++)
		{
		  sum += pow (L[beta][gamma], 2);
		}
	       L[alpha][beta] = sqrt (L[alpha][beta] - sum);
	    }
	  else if (alpha > beta)
	    {
	      for (lambda = 0; lambda < beta - 1; lambda++)
		{
		  sum1 += L[alpha][lambda] * L[lambda][beta];
		}
	      L[alpha][beta] = (L[alpha][beta] - sum1) / L[beta][beta];
	    }
	  else
	    {
	       L[alpha][beta] = 0;
	    }
	}
      
    }
	
}

  double ** createMat (int nbRow, int nbCol)
  {
    double **matrice, *temp_mat;
    int initcol, countr, countc;
    matrice = malloc (nbRow * sizeof (double));
    temp_mat = malloc (nbRow * nbCol * sizeof (double));
    for (initcol = 0; initcol < nbRow; initcol++)
      {
	matrice[initcol] = &temp_mat[initcol * nbCol];
      }
    for (countr = 0; countr < nbRow; countr++)
      {
	for (countc = 0; countc < nbCol; countc++)
	  {
	    matrice[countr][countc] = 0;
	  }
      }
    return matrice;
  }

 void destroyMat(double **mat1,int row,int col)
{
 free(mat1[0]);
 free(mat1);
}
  double **init_SF (int map_length, int map_height, double decor_dist,
		    double variance)
  {

    double **SF = createMat (map_length, map_height);
    double **unc_SF = createMat (map_length, map_height);
    double **L1 = createMat (2, 2);
    double **L2 = createMat (3, 3);
    double **L3 = createMat (4, 4);
    double **L4 = createMat (5, 5);

    double temp[2] = { 0.0 }, temp1[2] =
    {
    0.0}, temp2[3] =
    {
    0.0}, temp21[2] =
    {
    0.0}, temp3[4] =
    {
    0.0}, temp31[3] =
    {
    0.0}, temp4[5] =
    {
    0.0}, temp41[4] =
    {
    0.0}, temp211[3] =
    {
    0.0}, temp311[4] =
    {
    0.0}, temp411[5] =
    {
    0.0};
    int i, j, aa, b, c, d, k, l, m, bb, cc, dd, kk, ll, mm, ii, jj;

  /*double  L1[2][2] = {{1.0000,0.0},{0.9512,0.3085}};
       double  L2[3][3] = {{1.0000,0.0,0.0},{0.9512,0.3085,0},{0.9512,0.0872,0.2959}};
       double  L3[4][4] = {{1.0000,0.0,0.0,0.0},{0.9512,0.3085,0.0,0.0},{0.9512,0.0872,0.2959 ,0.0},{0.9317,0.2105,0.1574,0.2506}};
       double  L4[5][5] = {{1.0000,0.0,0.0,0.0,0.0},{0.9512,0.3085,0.0,0.0,0.0},{ 0.9048,0.2934,0.3085,0.0,0.0},{0.9512,0.0872,0.0257,0.2948,0.0},{0.9317,0.2105,0.0872, 0.1504,0.2394}};
     */

    double r = exp (-1 / decor_dist);	// decorrelation factor mostly equal to 20
    double r1 = pow (r, sqrt (2));
    double r2 = pow (r, 2);
    double r3 = pow (r, sqrt (5));

    double L11[2][2] = { {1, r}, {r, 1} };
    double L21[3][3] = { {1, r, r}, {r, 1, r1}, {r, r1, 1} };
    double L31[4][4] = { {1, r, r, r1}, {r, 1, r1, r}, {r, r1, 1, r}, {r1, r, r, 1} };
    double L41[5][5] = { {1, r, r2, r, r1}, {r, 1, r, r1, r}, {r2, r, 1, r3, r1}, {r, r1,r3, 1, r},
			 {r1, r, r1, r, 1} };

    L1[0][0] = L11[0][0];
    L1[0][1] = L11[0][1];
    L1[1][0] = L11[1][0];
    L1[1][1] = L11[1][1];
   /*for (bb = 0; bb < 2; bb++)
      {
	for (cc = 0; cc < 2; cc++)
	  {
	//Lt1[bb*2+cc] = L11[bb][cc];
	L1[bb][cc]=L11[bb][cc];
	  }
      }*/
    for (dd = 0; dd < 3; dd++)
      {
	for (kk = 0; kk < 3; kk++)
	  {
	L2[dd][kk]=L21[dd][kk];
	  }
      }


    for (ll = 0; ll < 4; ll++)
      {
	for (mm = 0; mm < 4; mm++)
	  {
	  L3[ll][mm]=L31[ll][mm];
	  }
      }


    for (ii = 0; ii < 5; ii++)
      {
	for (jj = 0; jj < 5; jj++)
	  {
	L4[ii][jj]=L41[ii][jj];
	  }
      }

  clapack_dpotrf(CblasRowMajor,CblasLower,2,&L1[0][0],2);
  clapack_dpotrf(CblasRowMajor,CblasLower,3,&L2[0][0],3);
  clapack_dpotrf(CblasRowMajor,CblasLower,4,&L3[0][0],4);
  clapack_dpotrf(CblasRowMajor,CblasLower,5,&L4[0][0],5);
    randominit (0);
    for (i = 0; i < map_length; i++)
      {
	for (j = 0; j < map_height; j++)
	  unc_SF[i][j] = gaussdouble (0.0, variance);
      }
    SF[0][0] = unc_SF[0][0];
    printf ("%f", SF[0][0]);
    for (aa = 0; aa < 2; aa++)
      {
	temp[aa] = L1[1][aa];
      }
    for (k = 1; k < map_height; k++)
      {
	temp1[0] = SF[0][k - 1];
	temp1[1] = unc_SF[0][k];
	SF[0][k] = cblas_ddot (2, temp, 1, temp1, 1);
      }
    
    for (b = 0; b < 3; b++)
      {
	temp2[b] = L2[2][b];
      }

    for (c = 0; c < 4; c++)
      {
	temp3[c] = L3[3][c];
      }
    
    for (d = 0; d < 5; d++)
      {
	temp4[d] = L4[4][d];
      }
    for (l = 1; l < map_length; l++)
      {
	for (m = 0; m < map_height; m++)
	  {
	    if (m == 0)
	      {
		temp21[0] = SF[l - 1][m];
		temp21[1] = SF[l - 1][m + 1];
		cblas_dtrsv (CblasRowMajor, CblasLower, CblasNoTrans,
			     CblasNonUnit, 2, &L1[0][0], 2, temp21, 1);

		temp211[0] = temp21[0];
		temp211[1] = temp21[1];
		temp211[2] = unc_SF[l][m];

		SF[l][m] = cblas_ddot (3, temp2, 1, temp211, 1);

	      }
	    else if (m == map_height - 1)
	      {
		temp31[0] = SF[l - 1][m - 1];
		temp31[1] = SF[l - 1][m];
		temp31[2] = SF[l][m - 1];

		cblas_dtrsv (CblasRowMajor, CblasLower, CblasNoTrans,
			     CblasNonUnit, 3, &L2[0][0], 3, temp31, 1);

		temp311[0] = temp31[0];
		temp311[1] = temp31[1];
		temp311[2] = temp31[2];
		temp311[3] = unc_SF[l][m];

		SF[l][m] = cblas_ddot (4, temp3, 1, temp311, 1);

	      }
	    else
	      {
		temp41[0] = SF[l - 1][m - 1];
		temp41[1] = SF[l - 1][m];
		temp41[2] = SF[l - 1][m + 1];
		temp41[3] = SF[l][m - 1];

		cblas_dtrsv (CblasRowMajor, CblasLower, CblasNoTrans,
			     CblasNonUnit, 4, &L3[0][0], 4, temp41, 1);

		temp411[0] = temp41[0];
		temp411[1] = temp41[1];
		temp411[2] = temp41[2];
		temp411[3] = temp41[3];
		temp411[4] = unc_SF[l][m];

		SF[l][m] = cblas_ddot (5, temp4, 1, temp411, 1);

	      }
	  }
      }
    
    return SF;
    destroyMat(unc_SF,map_length, map_height);
    destroyMat(L1,2,2);
    destroyMat(L2,3,3);
    destroyMat(L3,4,4);
    destroyMat(L4,5,5);

  }
