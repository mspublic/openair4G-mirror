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
	
  set_taus_seed(seed);
  LOG_I(OTG,"set taus seed to %d done \n", seed);
		
}

double uniform_rng() {		
  double random;
  random = (double)taus(OTG)/((double)0xffffffff);
  //LOG_D(OTG,"Uniform taus random number= %lf\n", random);
return random;
}






// Uniform Distribution using the Uniform_Random_Number_Generator

double uniform_dist(int min, int max) {
  double uniform_rn;
  uniform_rn = (max - min) * uniform_rng() + min;
  //LOG_D(OTG,"Uniform Random Nb = %lf, (min %d, max %d)\n", uniform_rn, min, max);	
  return uniform_rn;
}

// Gaussian Distribution using Box-Muller Transformation

double gaussian_dist(double mean, double std_dev) {
	double x_rand1,x_rand2, w, gaussian_rn_1;

	do {
		do {
			x_rand1 = 2.0 * uniform_rng() - 1;
			x_rand2 = 2.0 * uniform_rng() - 1;
			w = x_rand1 * x_rand1 + x_rand2 * x_rand2;
		} while (w >= 1.0);	
		w = sqrt((-2.0 * log(w)) / w);	
		gaussian_rn_1 = (std_dev * (x_rand1 * w)) + mean;
	} while (gaussian_rn_1 <= 0);
	LOG_D(OTG,"Gaussian Random Nb= %lf (mean %lf, deviation %lf)\n", gaussian_rn_1, mean, std_dev);
		
	return gaussian_rn_1;

}

// Exponential Distribution using the standard natural logarithmic transformations

double exponential_dist(double lambda) {	

	double exponential_rn;

	if (log(uniform_rng()) > 0)
		exponential_rn = log(uniform_rng()) / lambda;
	else
		exponential_rn = -log(uniform_rng()) / lambda;
	LOG_D(OTG,"Exponential Random Nb = %lf (lambda %lf)\n", exponential_rn, lambda);
	return exponential_rn;
}

// Poisson Distribution using Knuths Algorithm

double poisson_dist(double lambda){
	double poisson_rn, L, p, u;
	int k = 0;
	p = 1;
	L = exp(-lambda);
	do {
		u = uniform_rng();
		p = p * u;
		k += 1;
	} while (p > L);
	poisson_rn = k - 1;
	LOG_D(OTG,"Poisson Random Nb = %lf (lambda %lf)\n", poisson_rn, lambda);
	return poisson_rn;  

}




double weibull_dist(double scale, double shape){
	double weibull_rn;

	if ((scale<=0)||(shape<=0)){
		LOG_W(OTG,"Weibull :: scale=%.2f or shape%.2f <0 , adjust to new values: sale=3, shape=4 \n", scale,shape);
		scale=3;
		shape=4;
	}
	weibull_rn=scale * pow(-log(1-uniform_rng()), 1/shape);	
	LOG_D(OTG,"Weibull Random Nb = %lf (scale=%.2f, shape=%.2f)\n", weibull_rn, scale,shape);
	return weibull_rn; 

}

double pareto_dist(double scale, double shape) {
double pareto_rn;
	if ((scale<=0)||(shape<=0)){
		LOG_W(OTG,"Pareto :: scale=%.2f or shape%.2f <0 , adjust new values: sale=3, shape=4 \n", scale,shape);
		scale=3;
		shape=4;
	}
	pareto_rn=scale * pow(1/(1-uniform_rng()), 1/shape);
	LOG_D(OTG,"Pareto Random Nb = %lf (scale=%.2f, shape=%.2f)\n", pareto_rn,scale,shape);	
	return pareto_rn; 
}

double gamma_dist(double scale, double shape) {

double gamma_rn, mult_var=1;
int i, shape_int;

shape_int=ceil(shape);
	if ((scale<=0)||(shape_int<=0)){
		LOG_W(OTG,"Gamma :: scale=%.2f or shape%.2f <0 , adjust to new values: sale=0.5, shape=25 \n", scale,shape);
		scale=0.5;
		shape=25;
	}
	
	for(i=1;i<=shape_int;i++){ 
	  mult_var=mult_var*uniform_rng();
	}
	
	gamma_rn= (-1/scale)*log(mult_var);
	LOG_D(OTG,"Gamma Random Nb = %lf (scale=%.2f, shape=%.2f)\n", gamma_rn, scale, shape);	
	return gamma_rn;

}

double cauchy_dist(double scale, double shape ) {
double cauchy_rn;
	if ((scale<=0)||(shape<=0)){
		LOG_W(OTG,"Cauchy :: scale=%.2f or shape%.2f <0 , new values: sale=2, shape=10 \n", scale,shape);
		scale=2;
		shape=10;
	}


	cauchy_rn= scale*tan(PI*(uniform_rng()-0.5)) + shape;
	
	if (cauchy_rn<0){
	  cauchy_rn=fabs(cauchy_rn);  
	  LOG_D(OTG,"Cauchy Random Nb = %lf <0 (scale=%.2f, shape=%.2f), we use absolute value\n", cauchy_rn, scale, shape);
	}
	else
	  LOG_D(OTG,"Cauchy Random Nb = %lf (scale=%.2f, shape=%.2f)\n", cauchy_rn, scale, shape);
	return cauchy_rn;

}


double lognormal_dist(double mean, double std_dev){
  double lognormal_rn;
  lognormal_rn= exp(gaussian_dist(mean, std_dev));
return lognormal_rn;
}



double wichman_hill() { // not used

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
	random = temp - (int)temp;

	return random;
}


