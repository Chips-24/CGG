#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

/*************************************************************************
* *


This code computes the number of zeros on the critical line of the Zeta function.
https://en.wikipedia.org/wiki/Riemann_zeta_function 

This is one of the most important and non resolved problem in mathematics : https://www.science-et-vie.com/article-magazine/voici-les-7-plus-grands-problemes-de-mathematiques-jamais-resolus

This problem has been the subject of one of the most important distributed computing project in the cloud : more than 10000 machines during 2 years. 
They used this algorithm: very well optimized.
This project failed, bitten by a smaller team that used a far better algorithm. 
The code is based on the Thesis of Glendon Ralph Pugh (1992) : https://web.viu.ca/pughg/thesis.d/masters.thesis.pdf

We can optimize the code in numerous ways, and obviously parallelize it. 

Remark: we do not compute the zeros: we count them to check that they are on the Riemann Line.
Remark: Andrew Odlyzko created a method that is far more efficient but too complex to be the subject of an algorithmetical tuning exercice. 

The exercise is to sample a region on the critical line to count how many times the function changes sign, so that there is at least 1 zero between 2 sampling points.
Here we use a constant sampling but you can recode entirely the way to proceed.

Only a correct (right) count matters, and the performance.

compile g++ RiemannSiegel.cpp -O -o RiemannSiegel
--------------------------------------------------------------------------
./RiemannSiegel 10 1000 100
I found 649 Zeros in 3.459 seconds     # OK 
--------------------------------------------------------------------------
./RiemannSiegel 10 10000 10 
I found 10142 Zeros in 0.376 seconds     # OK
--------------------------------------------------------------------------
./RiemannSiegel 10 100000 10
I found 137931 Zeros in 6.934 seconds    # INCORRECT
--------------------------------------------------------------------------
./RiemannSiegel 10 100000 100
I found 138069 Zeros in 56.035 seconds   # OK
--------------------------------------------------------------------------
RiemannSiegel 10 1000000     need to find : 1747146     zeros
RiemannSiegel 10 10000000    need to find : 21136125    zeros
RiemannSiegel 10 100000000   need to find : 248888025   zeros
RiemannSiegel 10 1000000000  need to find : 2846548032  zeros
RiemannSiegel 10 10000000000 need to find : 32130158315 zeros


The more regions you validate and with the best timing, the more points you get.

The official world record of the zeros computed is 10^13 but with some FFTs and the method from Odlyzsko.
Compute time 1 year-core so an algortihm 10000*2*40 times more efficient than ZetaGrid's one. 

* *
*************************************************************************/

typedef unsigned long      ui32;
typedef unsigned long long ui64;

double dml_micros()
{
        static struct timezone tz;
        static struct timeval  tv;
        gettimeofday(&tv,&tz);
        return((tv.tv_sec*1000000.0)+tv.tv_usec);
}

int even(int n)
{
	if (n%2 == 0) return(1);
	else          return(-1);
}

double theta(double t)
{
	const double pi = 3.1415926535897932385;
	double t_div_2 	= t*0.5;
	double pawt2 	= t*t;
	double pawt3 	= pawt2*t;
	double pawt5 	= pawt3*pawt2;
	double pawt7 	= pawt5*pawt2;
	double pawt9 	= pawt7*pawt2;

	// return(  t/2.0*log(t/2.0/pi)  -  t/2.0  -  pi/8.0  + 1.0/48.0/t + 7.0/5760.0/pow(t,3.0) + 31.0/80640.0/powl(t,5.0) +127.0/430080.0/powl(t,7.0)+511.0/1216512.0/powl(t,9.0));
	return(   t_div_2*log(t_div_2/pi)- t_div_2 - pi*0.125 + 1.0/48.0/t + 7.0/5760.0/pawt3      + 31.0/80640.0/pawt5       +127.0/430080.0/pawt7      +511.0/1216512.0/pawt9      );
	//https://oeis.org/A282898  // numerators
	//https://oeis.org/A114721  // denominators
}


double C(int n, double z){
	double paw2 	= z*z;		
	double paw4 	= paw2*paw2;
	double paw6 	= paw4*paw2;
	double paw8 	= paw4*paw4;
	double paw16 	= paw8*paw8;
	double paw24 	= paw16*paw8;
	double paw32 	= paw16*paw16;
	double paw40 	= paw32*paw8;

    double paw3     = paw2*z;
    double paw5     = paw4*z;
    double paw7     = paw6*z;
    double paw9     = paw8*z;
    double paw17    = paw16*z;
    double paw25    = paw24*z;
    double paw33    = paw32*z;
    double paw41    = paw40*z;

	if (n==0)
		return(.38268343236508977173                	//pow(z, 0.0)
		      +.43724046807752044936 * paw2         	//pow(z, 2.0)
		      +.13237657548034352332 * paw4         	//pow(z, 4.0)
		      -.01360502604767418865 * paw6         	//pow(z, 6.0)
		      -.01356762197010358089 * paw8         	//pow(z, 8.0)
		      -.00162372532314446528 * paw8*paw2    	//pow(z,10.0)
		      +.00029705353733379691 * paw8*paw4    	//pow(z,12.0)
		      +.00007943300879521470 * paw8*paw6    	//pow(z,14.0)
		      +.00000046556124614505 * paw16        	//pow(z,16.0)
		      -.00000143272516309551 * paw16*paw2   	//pow(z,18.0)
		      -.00000010354847112313 * paw16*paw4   	//pow(z,20.0)
		      +.00000001235792708386 * paw16*paw6   	//pow(z,22.0)
		      +.00000000178810838580 * paw24        	//pow(z,24.0)
		      -.00000000003391414390 * paw24*paw2   	//pow(z,26.0)
		      -.00000000001632663390 * paw24*paw4   	//pow(z,28.0)
		      -.00000000000037851093 * paw24*paw6   	//pow(z,30.0)
		      +.00000000000009327423 * paw32        	//pow(z,32.0)
		      +.00000000000000522184 * paw32*paw2   	//pow(z,34.0)
		      -.00000000000000033507 * paw32*paw4   	//pow(z,36.0)
		      -.00000000000000003412 * paw32*paw6   	//pow(z,38.0)
		      +.00000000000000000058 * paw40        	//pow(z,40.0)
		      +.00000000000000000015 * paw40*paw2); 	//pow(z,42.0));
	else if (n==1)   
		return(-.02682510262837534703 * z           	//pow(z, 1.0)
		       +.01378477342635185305 * paw3        	//pow(z, 3.0)
		       +.03849125048223508223 * paw5        	//pow(z, 5.0)
		       +.00987106629906207647 * paw7        	//pow(z, 7.0)
		       -.00331075976085840433 * paw9        	//pow(z, 9.0)
		       -.00146478085779541508 * paw9*paw2   	//pow(z,11.0)
		       -.00001320794062487696 * paw9*paw4   	//pow(z,13.0)
		       +.00005922748701847141 * paw9*paw6   	//pow(z,15.0)
		       +.00000598024258537345 * paw17       	//pow(z,17.0)
		       -.00000096413224561698 * paw17*paw2  	//pow(z,19.0)
		       -.00000018334733722714 * paw17*paw4  	//pow(z,21.0)
		       +.00000000446708756272 * paw17*paw6  	//pow(z,23.0)
		       +.00000000270963508218 * paw25       	//pow(z,25.0)
		       +.00000000007785288654 * paw25*paw2  	//pow(z,27.0)
		       -.00000000002343762601 * paw25*paw4  	//pow(z,29.0)
		       -.00000000000158301728 * paw25*paw6  	//pow(z,31.0)
		       +.00000000000012119942 * paw33       	//pow(z,33.0)
		       +.00000000000001458378 * paw33*paw2  	//pow(z,35.0)
		       -.00000000000000028786 * paw33*paw4  	//pow(z,37.0)
		       -.00000000000000008663 * paw33*paw6  	//pow(z,39.0)
		       -.00000000000000000084 * paw41       	//pow(z,41.0)
		       +.00000000000000000036 * paw41*paw2  	//pow(z,43.0)
		       +.00000000000000000001 * paw41*paw4);	//pow(z,45.0));
	else if (n==2)
		return(+.00518854283029316849               	//pow(z, 0.0)
		       +.00030946583880634746 * paw2        	//pow(z, 2.0)
		       -.01133594107822937338 * paw4        	//pow(z, 4.0)
		       +.00223304574195814477 * paw6        	//pow(z, 6.0)
		       +.00519663740886233021 * paw8        	//pow(z, 8.0)
		       +.00034399144076208337 * paw8*paw2   	//pow(z,10.0)
		       -.00059106484274705828 * paw8*paw4   	//pow(z,12.0)
		       -.00010229972547935857 * paw8*paw6   	//pow(z,14.0)
		       +.00002088839221699276 * paw16       	//pow(z,16.0)
		       +.00000592766549309654 * paw16*paw2  	//pow(z,18.0)
		       -.00000016423838362436 * paw16*paw4  	//pow(z,20.0)
		       -.00000015161199700941 * paw16*paw6  	//pow(z,22.0)
		       -.00000000590780369821 * paw24       	//pow(z,24.0)
		       +.00000000209115148595 * paw24*paw2  	//pow(z,26.0)
		       +.00000000017815649583 * paw24*paw4  	//pow(z,28.0)
		       -.00000000001616407246 * paw24*paw6  	//pow(z,30.0)
		       -.00000000000238069625 * paw32       	//pow(z,32.0)
		       +.00000000000005398265 * paw32*paw2  	//pow(z,34.0)
		       +.00000000000001975014 * paw32*paw4  	//pow(z,36.0)
		       +.00000000000000023333 * paw32*paw6  	//pow(z,38.0)
		       -.00000000000000011188 * paw40       	//pow(z,40.0)
		       -.00000000000000000416 * paw40*paw2  	//pow(z,42.0)
		       +.00000000000000000044 * paw40*paw4  	//pow(z,44.0)
		       +.00000000000000000003 * paw40*paw6);	//pow(z,46.0));
	else if (n==3)
		return(-.00133971609071945690 * z           	//pow(z, 1.0)
		       +.00374421513637939370 * paw3        	//pow(z, 3.0)
		       -.00133031789193214681 * paw5        	//pow(z, 5.0)
		       -.00226546607654717871 * paw7        	//pow(z, 7.0)
		       +.00095484999985067304 * paw9        	//pow(z, 9.0)
		       +.00060100384589636039 * paw9*paw2   	//pow(z,11.0)
		       -.00010128858286776622 * paw9*paw4   	//pow(z,13.0)
		       -.00006865733449299826 * paw9*paw6   	//pow(z,15.0)
		       +.00000059853667915386 * paw17       	//pow(z,17.0)
		       +.00000333165985123995 * paw17*paw2  	//pow(z,19.0)
		       +.00000021919289102435 * paw17*paw4  	//pow(z,21.0)
		       -.00000007890884245681 * paw17*paw6  	//pow(z,23.0)
		       -.00000000941468508130 * paw25       	//pow(z,25.0)
		       +.00000000095701162109 * paw25*paw2  	//pow(z,27.0)
		       +.00000000018763137453 * paw25*paw4  	//pow(z,29.0)
		       -.00000000000443783768 * paw25*paw6  	//pow(z,31.0)
		       -.00000000000224267385 * paw33       	//pow(z,33.0)
		       -.00000000000003627687 * paw33*paw2  	//pow(z,35.0)
		       +.00000000000001763981 * paw33*paw4  	//pow(z,37.0)
		       +.00000000000000079608 * paw33*paw6  	//pow(z,39.0)
		       -.00000000000000009420 * paw41       	//pow(z,41.0)
		       -.00000000000000000713 * paw41*paw2  	//pow(z,43.0)
		       +.00000000000000000033 * paw41*paw4  	//pow(z,45.0)
		       +.00000000000000000004 * paw41*paw6);	//pow(z,47.0));
	else
		return(+.00046483389361763382               	//pow(z, 0.0)
		       -.00100566073653404708 * paw2        	//pow(z, 2.0)
		       +.00024044856573725793 * paw4        	//pow(z, 4.0)
		       +.00102830861497023219 * paw6        	//pow(z, 6.0)
		       -.00076578610717556442 * paw8        	//pow(z, 8.0)
		       -.00020365286803084818 * paw8*paw2   	//pow(z,10.0)
		       +.00023212290491068728 * paw8*paw4   	//pow(z,12.0)
		       +.00003260214424386520 * paw8*paw6   	//pow(z,14.0)
		       -.00002557906251794953 * paw16       	//pow(z,16.0)
		       -.00000410746443891574 * paw16*paw2  	//pow(z,18.0)
		       +.00000117811136403713 * paw16*paw4  	//pow(z,20.0)
		       +.00000024456561422485 * paw16*paw6  	//pow(z,22.0)
		       -.00000002391582476734 * paw24       	//pow(z,24.0)
		       -.00000000750521420704 * paw24*paw2  	//pow(z,26.0)
		       +.00000000013312279416 * paw24*paw4  	//pow(z,28.0)
		       +.00000000013440626754 * paw24*paw6  	//pow(z,30.0)
		       +.00000000000351377004 * paw32       	//pow(z,32.0)
		       -.00000000000151915445 * paw32*paw2  	//pow(z,34.0)
		       -.00000000000008915418 * paw32*paw4  	//pow(z,36.0)
		       +.00000000000001119589 * paw32*paw6  	//pow(z,38.0)
		       +.00000000000000105160 * paw40       	//pow(z,40.0)
		       -.00000000000000005179 * paw40*paw2  	//pow(z,42.0)
		       -.00000000000000000807 * paw40*paw4  	//pow(z,44.0)
		       +.00000000000000000011 * paw40*paw6  	//pow(z,46.0)
		       +.00000000000000000004 * paw40*paw8);	//pow(z,48.0));
}

double Z(double t, int n)
//*************************************************************************
// Riemann-Siegel Z(t) function implemented per the Riemenn Siegel formula.
// See http://mathworld.wolfram.com/Riemann-SiegelFormula.html for details
//*************************************************************************
{
	double p; /* fractional part of sqrt(t/(2.0*pi))*/
	double C(int,double); /* coefficient of (2*pi/t)^(k*0.5) */
	const double pi = 3.1415926535897932385; 
	int N = sqrt(t/(2.0 * pi)); 
	p = sqrt(t/(2.0 * pi)) - N; 
	double tt = theta(t); 
	double ZZ = 0.0; 
	for (int j=1;j <= N;j++) {
		ZZ = ZZ + 1.0/sqrt((double) j ) * cos(fmod(tt -t*log((double) j),2.0*pi));
	} 
	ZZ = 2.0 * ZZ; 
	double R  = 0.0; 
	for (int k=0;k <= n;k++) {
		R = R + C(k,2.0*p-1.0) * pow(2.0*pi/t, ((double) k)*0.5);
	} 
	R = even(N-1) * pow(2.0 * pi / t,0.25) * R; 
	return(ZZ + R);
}

int main(int argc,char **argv)
{
	double LOWER,UPPER,SAMP;
	const double pi = 3.1415926535897932385;
	
	if(argc!=4){
		printf("usage : %s LOWER UPPER SAMP\n",argv[0]);
		exit(0);
	}
	LOWER=atof(argv[1]);
	UPPER=atof(argv[2]);
	SAMP=atof(argv[3]);
	if (LOWER<0.0 || UPPER<0.0){
		printf("LOWER and UPPER must be positive\n");
		exit(0);
	}
	if (LOWER>UPPER){
		printf("LOWER must be lower than UPPER\n");
		exit(0);
	}
	if (SAMP<1.0){
		printf("SAMP must be superior or equal to 1.0\n");
		exit(0);
	}


	double estimate_zeros=theta(UPPER)/pi;
	printf("I estimate I will find %1.3lf zeros\n",estimate_zeros);

	double STEP = 1.0/SAMP;
	// ui64   NUMSAMPLES=floor((UPPER-LOWER)*SAMP+1.0);
	double prev=0.0;
	double count=0.0;
	double t1=dml_micros();
	for (double t=LOWER;t<=UPPER;t+=STEP){
		double zout=Z(t,4);
		if(t>LOWER){
			if(   ((zout<0.0)&&(prev>0.0))
				||((zout>0.0)&&(prev<0.0))){
				//printf("%20.6lf  %20.12lf %20.12lf\n",t,prev,zout);
				count++;
			}
		}
		prev=zout;
	}
	double t2=dml_micros();

	printf("I found %1.0lf Zeros in %.3lf seconds\n",count,(t2-t1)/1000000.0);

	return(0);
}
