#include<stdio.h>
#include<math.h>
#include<cblas.h>
//#include<atlas_lapack.h>
#include<clapack.h>
#include<oaisim.h>

double **init_SF (int map_length, int map_height, double decor_dist, double variance);	//,double decor,double var);
double **createMat (int nbRow, int nbCol);
void destroyMat(double **mat,int row,int col);
void cholDec (double **L, int nbr, int nbc);
void displayMat(double **m, int nbr1, int nbc1);
