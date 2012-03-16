/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file oml.c
* \brief Data structure for distribution libraries
* \author N. Nikaein and A. Hafsaoui
* \date 2011
* \version 1.0
* \company Eurecom
* \email: openair_tech@eurecom.fr
* \note
* \warning
*/


#include <stdio.h>
#include <math.h>
#include <errno.h>

#include "oml.h"


static int x, y, z;


void init_seeds(int seed){
	
	LOG_I(OTG,"seeds:%d\n", seed);	
	srand(seed*0xa2e489f2);
	// Random number between 1 and 30000 is created
	x = (( rand() % 30000) + 1);
	y = (( rand() % 30000) + 1);
	z = (( rand() % 30000) + 1);
	LOG_I(OTG,"Initial seeds: x = %d, y = %d, z = %d,\n", x,y,z);
	set_taus_seed(seed);
		
}

double uniform_rng() {		
	double random;

	random = wichman_hill();
        LOG_I(OTG,"Uniform random number using wichman_hill= %lf\n", random);
return random;
}




double wichman_hill() {

		double temp, random;
		int modx = x % 177;
		int mody = y % 176;
		int modz = z % 178;
	
	//First  Generator	
	x = (171 * modx) - (2 * modx);
	if (x < 0) 
	x += 30269;
	//Second  Generator
	y = (172 * mody) - (35 * mody);
	if (y < 0) 
	y += 30307;
	//Third Generator
	z = (170 * modz) - (63 * modz);
	if (z < 0) 
	z += 30323;
	temp = (x / 30269.0) + (y / 30307.0) + (z / 30323.0);
        //printf ("temp %lf\n", temp);
	random = temp - (int)temp;

	return random;
}


// Uniform Distribution using the Uniform_Random_Number_Generator

double uniform_dist(int min, int max) {
	LOG_I(OTG,"Uniform :: MIN = %d\n", min);
	LOG_I(OTG,"Uniform :: MAX = %d\n", max);
	double uniform_rn;
        uniform_rn = (max - min) * uniform_rng() + min;
        LOG_I(OTG,"Uniform Random Nb = %lf\n", uniform_rn);	
	return uniform_rn;
}

// Gaussian Distribution using Box-Muller Transformation

double gaussian_dist(double mean, double std_dev) {
	double x_rand1,x_rand2, w, gaussian_rn_1, gaussian_rn_2;

	LOG_I(OTG,"Gaussian mean= %lf and std deviation= %lf \n", mean, std_dev);
	do {
		do {
			x_rand1 = 2.0 * uniform_rng() - 1;
			x_rand2 = 2.0 * uniform_rng() - 1;
			w = x_rand1 * x_rand1 + x_rand2 * x_rand2;
		} while (w >= 1.0);	
		w = sqrt((-2.0 * log(w)) / w);	
		gaussian_rn_1 = (std_dev * (x_rand1 * w)) + mean;
		gaussian_rn_2 = (std_dev * (x_rand2 * w)) + mean;
	} while (gaussian_rn_1 <= 0);
	LOG_I(OTG,"Gaussian Random Nb= %lf\n", gaussian_rn_1);
		
	return gaussian_rn_1;

}

// Exponential Distribution using the standard natural logarithmic transformations

double exponential_dist(double lambda)
{	

	double exponential_rn;

LOG_I(OTG,"Exponential lambda= %lf\n", lambda);

	if (log(uniform_rng()) > 0)
		exponential_rn = log(uniform_rng()) / lambda;
	else
		exponential_rn = -log(uniform_rng()) / lambda;
	printf ("Exponential Random Nb = %lf \n", exponential_rn);
	return exponential_rn;
}

// Poisson Distribution using Knuths Algorithm

double poisson_dist(double lambda)
{
	double poisson_rn, L, p, u;
	int k = 0;
	p = 1;
	L = exp(-lambda);

	LOG_I(OTG,"Poisson lambda= %lf\n", lambda);

	do {
		u = uniform_rng();
		p = p * u;
		k += 1;
	} while (p > L);
	poisson_rn = k - 1;
	LOG_I(OTG,"Poisson Random Nb = %lf \n", poisson_rn);
	return poisson_rn;  

}




double weibull_dist(double scale, double shape)
{
	double weibull_rn;

	if ((scale<=0)||(shape<=0)){
		LOG_I(OTG,"Weibull :: scale=%.2f or shape%.2f <0 , new values: sale=3, shape=4 \n", scale,shape);
		scale=3;
		shape=4;
	}
	LOG_I(OTG,"Weibull :: scale=%.2f, shape=%.2f \n", scale,shape);
	weibull_rn=scale * pow(-log(1-uniform_rng()), 1/shape);	
	LOG_I(OTG,"Weibull Random Nb = %lf \n", weibull_rn);
	return weibull_rn; 

}

double pareto_dist(double scale, double shape)
{
double pareto_rn;
	if ((scale<=0)||(shape<=0)){
		LOG_I(OTG,"Pareto :: scale=%.2f or shape%.2f <0 , new values: sale=3, shape=4 \n", scale,shape);
		scale=3;
		shape=4;
	}
	LOG_I(OTG,"Pareto :: scale=%.2f, shape=%.2f \n", scale,shape);
	pareto_rn=scale * pow(1/(1-uniform_rng()), 1/shape);
	LOG_I(OTG,"Pareto Random Nb = %lf \n", pareto_rn);	
	return pareto_rn; 
}

double gamma_dist(double scale, double shape)
{

double gamma_rn, mult_var=1;
int i, shape_int;

shape_int=ceil(shape);
	if ((scale<=0)||(shape_int<=0)){
		LOG_I(OTG,"Gamma :: scale=%.2f or shape%.2f <0 , new values: sale=0.5, shape=25 \n", scale,shape);
		scale=0.5;
		shape=25;
	}
	
LOG_I(OTG,"Gamma :: scale=%.2f, shape=%.2f \n", scale,shape);

	for(i=1;i<=shape_int;i++)
	{ mult_var=mult_var*uniform_rng();
	//LOG_I(OTG,"mult_var %lf \n",mult_var);
	}

	gamma_rn= (-1/scale)*log(mult_var);
	LOG_I(OTG,"Gamma Random Nb = %lf \n", gamma_rn);	
	return gamma_rn;

}

double cauchy_dist(double scale, double shape)
{
double cauchy_rn;
	if ((scale<=0)||(shape<=0)){
		LOG_I(OTG,"Cauchy :: scale=%.2f or shape%.2f <0 , new values: sale=2, shape=10 \n", scale,shape);
		scale=2;
		shape=10;
	}

LOG_I(OTG,"Cauchy :: scale=%.2f, shape=%.2f \n", scale,shape);

	cauchy_rn= scale*tan(PI*(uniform_rng()-0.5)) + shape;
	
	if (cauchy_rn<0)
	LOG_I(OTG,"Cauchy Random Nb = %lf <0, we use absolute value\n", cauchy_rn);
	
	cauchy_rn=fabs(cauchy_rn);
	LOG_I(OTG,"Cauchy Random Nb = %lf \n", cauchy_rn);
	return cauchy_rn;

}




