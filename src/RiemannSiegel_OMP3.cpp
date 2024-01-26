#include <stdio.h>
#include <stdlib.h>
#include <armpl.h>
#include <math.h>
#include <sys/time.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <complex>
#include <vector>
#include <cassert>
#include <omp.h>

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

std::vector<double> invert_sqrt;
std::vector<double> log_int;

double dml_micros()
{
        static struct timezone tz;
        static struct timeval  tv;
        gettimeofday(&tv,&tz);
        return((tv.tv_sec*1000000.0)+tv.tv_usec);
}

#define even(n) ( 1-2*((n)&1) )
// #define even(n) ( (n) % 2 ? -1 : 1 )
/*
int even(int n)
{
	if (n%2 == 0) return(1);
	else          return(-1);
}
*/

double theta(double t)
{
	const double pi = 3.1415926535897932385;
	long double pawt2 	= t*t;
	long double pawt3 	= pawt2*t;
	long double pawt5 	= pawt3*pawt2;
	long double pawt7 	= pawt5*pawt2;
	long double pawt9 	= pawt7*pawt2;
	// Chanhe div by mul
	// return(t/2.0*log(t/2.0/pi) - t/2.0 - pi/8.0   + 1.0/48.0/t + 7.0/5760.0/pow(t,3.0) + 31.0/80640.0/powl(t,5.0) +127.0/430080.0/powl(t,7.0)+511.0/1216512.0/powl(t,9.0));
	return(   t*0.5*log(t*0.5/pi) - t*0.5 - pi*0.125 + 1.0/48.0/t + 7.0/5760.0/pawt3      + 31.0/80640.0/pawt5       +127.0/430080.0/pawt7      +511.0/1216512.0/pawt9      );
	//https://oeis.org/A282898  // numerators
	//https://oeis.org/A114721  // denominators
}

void __attribute__((optimize("O1"))) compute_table(ui64 size)
{
	invert_sqrt.reserve(size);
	log_int.reserve(size);
	#pragma omp parallel for shared(invert_sqrt,log_int)
	for (ui64 k = 1; k < size; k++)
	{
		invert_sqrt[k] = 1.0/sqrt(k);
		log_int[k] = log(k);
	}
}

/*
 * Inlineing it as each if statement 
 *
double C(int n, double z){
	double paw2 	= z*z;
	double paw4 	= paw2*paw2;
	double paw6 	= paw4*paw2;
	double paw8 	= paw4*paw4;
	double paw16 	= paw8*paw8;
	double paw24 	= paw16*paw8;
	double paw32 	= paw16*paw16;
	double paw40 	= paw32*paw8;

	if (n==0) {
		return(  .38268343236508977173 * 1					// pow(z, 0.0)
				+.43724046807752044936 * paw2				// pow(z, 2.0)
				+.13237657548034352332 * paw4				// pow(z, 4.0)
				-.01360502604767418865 * paw6				// pow(z, 6.0)
				-.01356762197010358089 * paw8				// pow(z, 8.0)
				-.00162372532314446528 * paw8*paw2			// pow(z,10.0)
				+.00029705353733379691 * paw8*paw4			// pow(z,12.0)
				+.00007943300879521470 * paw8*paw6			// pow(z,14.0)
				+.00000046556124614505 * paw16				// pow(z,16.0)
				-.00000143272516309551 * paw16*paw2			// pow(z,18.0)
				-.00000010354847112313 * paw16*paw4			// pow(z,20.0)
				+.00000001235792708386 * paw16*paw6			// pow(z,22.0)
				+.00000000178810838580 * paw24				// pow(z,24.0)
				-.00000000003391414390 * paw24*paw2			// pow(z,26.0)
				-.00000000001632663390 * paw24*paw4			// pow(z,28.0)
				-.00000000000037851093 * paw24*paw6			// pow(z,30.0)
				+.00000000000009327423 * paw32				// pow(z,32.0)
				+.00000000000000522184 * paw32*paw2			// pow(z,34.0)
				-.00000000000000033507 * paw32*paw4			// pow(z,36.0)
				-.00000000000000003412 * paw32*paw6			// pow(z,38.0)
				+.00000000000000000058 * paw40				// pow(z,40.0)
				+.00000000000000000015 * paw40*paw2 );		// pow(z,42.0));
	}
	else if (n==1) {
		 return(-.02682510262837534703 * z					// pow(z, 1.0)
				+.01378477342635185305 * paw2*z				// pow(z, 3.0)
				+.03849125048223508223 * paw4*z				// pow(z, 5.0)
				+.00987106629906207647 * paw4*paw2*z		// pow(z, 7.0)
				-.00331075976085840433 * paw8*z				// pow(z, 9.0)
				-.00146478085779541508 * paw8*paw2*z		// pow(z,11.0)
				-.00001320794062487696 * paw8*paw4*z		// pow(z,13.0)
				+.00005922748701847141 * paw8*paw6*z		// pow(z,15.0)
				+.00000598024258537345 * paw16*z			// pow(z,17.0)
				-.00000096413224561698 * paw16*paw2*z		// pow(z,19.0)
				-.00000018334733722714 * paw16*paw4*z		// pow(z,21.0)
				+.00000000446708756272 * paw16*paw6*z		// pow(z,23.0)
				+.00000000270963508218 * paw24*z			// pow(z,25.0)
				+.00000000007785288654 * paw24*paw2*z		// pow(z,27.0)
				-.00000000002343762601 * paw24*paw4*z		// pow(z,29.0)
				-.00000000000158301728 * paw24*paw6*z		// pow(z,31.0)
				+.00000000000012119942 * paw32*z			// pow(z,33.0)
				+.00000000000001458378 * paw32*paw2*z		// pow(z,35.0)
				-.00000000000000028786 * paw32*paw4*z		// pow(z,37.0)
				-.00000000000000008663 * paw32*paw6*z		// pow(z,39.0)
				-.00000000000000000084 * paw40*z			// pow(z,41.0)
				+.00000000000000000036 * paw40*paw2*z		// pow(z,43.0)
				+.00000000000000000001 * paw40*paw4*z );	// pow(z,45.0));
	}
	else if (n==2) {
		 return(+.00518854283029316849 * 1					// pow(z, 0.0)
				+.00030946583880634746 * paw2				// pow(z, 2.0)
				-.01133594107822937338 * paw4				// pow(z, 4.0)
				+.00223304574195814477 * paw6				// pow(z, 6.0)
				+.00519663740886233021 * paw8				// pow(z, 8.0)
				+.00034399144076208337 * paw8*paw2			// pow(z,10.0)
				-.00059106484274705828 * paw8*paw4			// pow(z,12.0)
				-.00010229972547935857 * paw8*paw6			// pow(z,14.0)
				+.00002088839221699276 * paw16				// pow(z,16.0)
				+.00000592766549309654 * paw16*paw2			// pow(z,18.0)
				-.00000016423838362436 * paw16*paw4			// pow(z,20.0)
				-.00000015161199700941 * paw16*paw6			// pow(z,22.0)
				-.00000000590780369821 * paw24				// pow(z,24.0)
				+.00000000209115148595 * paw24*paw2			// pow(z,26.0)
				+.00000000017815649583 * paw24*paw4			// pow(z,28.0)
				-.00000000001616407246 * paw24*paw6			// pow(z,30.0)
				-.00000000000238069625 * paw32				// pow(z,32.0)
				+.00000000000005398265 * paw32*paw2			// pow(z,34.0)
				+.00000000000001975014 * paw32*paw4			// pow(z,36.0)
				+.00000000000000023333 * paw32*paw6			// pow(z,38.0)
				-.00000000000000011188 * paw40				// pow(z,40.0)
				-.00000000000000000416 * paw40*paw2			// pow(z,42.0)
				+.00000000000000000044 * paw40*paw4			// pow(z,44.0)
				+.00000000000000000003 * paw40*paw6 );		// pow(z,46.0));
	}
	else if (n==3) {
		 return(-.00133971609071945690 * z					// pow(z, 1.0)
				+.00374421513637939370 * paw2*z				// pow(z, 3.0)
				-.00133031789193214681 * paw4*z				// pow(z, 5.0)
				-.00226546607654717871 * paw4*paw2*z		// pow(z, 7.0)
				+.00095484999985067304 * paw8*z				// pow(z, 9.0)
				+.00060100384589636039 * paw8*paw2*z		// pow(z,11.0)
				-.00010128858286776622 * paw8*paw4*z		// pow(z,13.0)
				-.00006865733449299826 * paw8*paw6*z		// pow(z,15.0)
				+.00000059853667915386 * paw16*z			// pow(z,17.0)
				+.00000333165985123995 * paw16*paw2*z		// pow(z,19.0)
				+.00000021919289102435 * paw16*paw4*z		// pow(z,21.0)
				-.00000007890884245681 * paw16*paw6*z		// pow(z,23.0)
				-.00000000941468508130 * paw24*z			// pow(z,25.0)
				+.00000000095701162109 * paw24*paw2*z		// pow(z,27.0)
				+.00000000018763137453 * paw24*paw4*z		// pow(z,29.0)
				-.00000000000443783768 * paw24*paw6*z		// pow(z,31.0)
				-.00000000000224267385 * paw32*z			// pow(z,33.0)
				-.00000000000003627687 * paw32*paw2*z		// pow(z,35.0)
				+.00000000000001763981 * paw32*paw4*z		// pow(z,37.0)
				+.00000000000000079608 * paw32*paw6*z		// pow(z,39.0)
				-.00000000000000009420 * paw40*z			// pow(z,41.0)
				-.00000000000000000713 * paw40*paw2*z		// pow(z,43.0)
				+.00000000000000000033 * paw40*paw4*z		// pow(z,45.0)
				+.00000000000000000004 * paw40*paw6*z );	// pow(z,47.0));
	}
	else {
		 return(+.00046483389361763382 * 1					// pow(z, 0.0)
				-.00100566073653404708 * paw2				// pow(z, 2.0)
				+.00024044856573725793 * paw4				// pow(z, 4.0)
				+.00102830861497023219 * paw6				// pow(z, 6.0)
				-.00076578610717556442 * paw8				// pow(z, 8.0)
				-.00020365286803084818 * paw8*paw2			// pow(z,10.0)
				+.00023212290491068728 * paw8*paw4			// pow(z,12.0)
				+.00003260214424386520 * paw8*paw6			// pow(z,14.0)
				-.00002557906251794953 * paw16				// pow(z,16.0)
				-.00000410746443891574 * paw16*paw2			// pow(z,18.0)
				+.00000117811136403713 * paw16*paw4			// pow(z,20.0)
				+.00000024456561422485 * paw16*paw6			// pow(z,22.0)
				-.00000002391582476734 * paw24				// pow(z,24.0)
				-.00000000750521420704 * paw24*paw2			// pow(z,26.0)
				+.00000000013312279416 * paw24*paw4			// pow(z,28.0)
				+.00000000013440626754 * paw24*paw6			// pow(z,30.0)
				+.00000000000351377004 * paw32				// pow(z,32.0)
				-.00000000000151915445 * paw32*paw2			// pow(z,34.0)
				-.00000000000008915418 * paw32*paw4			// pow(z,36.0)
				+.00000000000001119589 * paw32*paw6			// pow(z,38.0)
				+.00000000000000105160 * paw40				// pow(z,40.0)
				-.00000000000000005179 * paw40*paw2			// pow(z,42.0)
				-.00000000000000000807 * paw40*paw4			// pow(z,44.0)
				+.00000000000000000011 * paw40*paw6			// pow(z,46.0)
				+.00000000000000000004 * paw40*paw8 );		// pow(z,48.0));
	}
}
*/
#define paws(z) 	double paw2 	= z*z;			\
					double paw4 	= paw2*paw2;	\
					double paw6 	= paw4*paw2;	\
					double paw8 	= paw4*paw4;	\
					double paw16 	= paw8*paw8;	\
					double paw24 	= paw16*paw8;	\
					double paw32 	= paw16*paw16;	\
					double paw40 	= paw32*paw8;   \
                    double paw1     = z;          	\
                    double paw3     = paw2*z;       \
                    double paw5     = paw4*z;       \
                    double paw7     = paw6*z;       \
                    double paw9     = paw8*z;       \
                    double paw17    = paw16*z;      \
                    double paw25    = paw24*z;      \
                    double paw33    = paw32*z;      \
                    double paw41    = paw40*z;      


#define C0  (	.38268343236508977173																																							\
				+( .43724046807752044936 -.00162372532314446528*paw8 -.00000143272516309551*paw16 -.00000000003391414390*paw24 +.00000000000000522184*paw32 +.00000000000000000015*paw40)* paw2	\
				+( .13237657548034352332 +.00029705353733379691*paw8 -.00000010354847112313*paw16 -.00000000001632663390*paw24 -.00000000000000033507*paw32)* paw4								\
				+(-.01360502604767418865 +.00007943300879521470*paw8 +.00000001235792708386*paw16 -.00000000000037851093*paw24 -.00000000000000003412*paw32)* paw6								\
				-.01356762197010358089 * paw8																																					\
				+.00000046556124614505 * paw16																																					\
				+.00000000178810838580 * paw24																																					\
				+.00000000000009327423 * paw32																																					\
				+.00000000000000000058 * paw40 )

#define C1 (	-.02682510262837534703 * paw1																																					\
				+.01378477342635185305 * paw3																																					\
				+.03849125048223508223 * paw5																																					\
				+.00987106629906207647 * paw7																																					\
				-.00331075976085840433 * paw9																																					\
				+(-.00146478085779541508*paw9 -.00000096413224561698*paw17 +.00000000007785288654*paw25 +.00000000000001458378*paw33 +.00000000000000000036*paw41)* paw2						\
				+(-.00001320794062487696*paw9 -.00000018334733722714*paw17 -.00000000002343762601*paw25 -.00000000000000028786*paw33 +.00000000000000000001*paw41)* paw4						\
				+( .00005922748701847141*paw9 +.00000000446708756272*paw17 -.00000000000158301728*paw25 -.00000000000000008663*paw33)* paw6  													\
				+.00000598024258537345 * paw17																																					\
				+.00000000270963508218 * paw25																																					\
				+.00000000000012119942 * paw33  																																				\
				-.00000000000000000084 * paw41 )

#define C2 (	+.00518854283029316849																																							\
				+( .00030946583880634746 +.00034399144076208337*paw8 +.00000592766549309654*paw16 +.00000000209115148595*paw24 +.00000000000005398265*paw32 -.00000000000000000416*paw40)* paw2	\
				+(-.01133594107822937338 -.00059106484274705828*paw8 -.00000016423838362436*paw16 +.00000000017815649583*paw24 +.00000000000001975014*paw32 +.00000000000000000044*paw40)* paw4	\
				+( .00223304574195814477 -.00010229972547935857*paw8 -.00000015161199700941*paw16 -.00000000001616407246*paw24 +.00000000000000023333*paw32 +.00000000000000000003*paw40)* paw6	\
				+.00519663740886233021 * paw8																																					\
				+.00002088839221699276 * paw16																																					\
				-.00000000590780369821 * paw24																																					\
				-.00000000000238069625 * paw32																																					\
				-.00000000000000011188 * paw40 )

#define C3 (	-.00133971609071945690 * paw1																																					\
				+.00374421513637939370 * paw3																																					\
				-.00133031789193214681 * paw5																																					\
				-.00226546607654717871 * paw7   																																				\
				+.00095484999985067304 * paw9																																					\
				+( .00060100384589636039*paw9 +.00000333165985123995*paw17 +.00000000095701162109*paw25 -.00000000000003627687*paw33 -.00000000000000000713*paw41)* paw2						\
				+(-.00010128858286776622*paw9 +.00000021919289102435*paw17 +.00000000018763137453*paw25 +.00000000000001763981*paw33 +.00000000000000000033*paw41)* paw4						\
				+(-.00006865733449299826*paw9 -.00000007890884245681*paw17 -.00000000000443783768*paw25 +.00000000000000079608*paw33 +.00000000000000000004*paw41)* paw6						\
				+.00000059853667915386 * paw17																																					\
				-.00000000941468508130 * paw25																																					\
				-.00000000000224267385 * paw33																																					\
				-.00000000000000009420 * paw41 )

#define C4 (	+.00046483389361763382 																																							\
				+(-.00100566073653404708 -.00020365286803084818*paw8 -.00000410746443891574*paw16 -.00000000750521420704*paw24 -.00000000000151915445*paw32 -.00000000000000005179*paw40)* paw2	\
				+( .00024044856573725793 +.00023212290491068728*paw8 +.00000117811136403713*paw16 +.00000000013312279416*paw24 -.00000000000008915418*paw32 -.00000000000000000807*paw40)* paw4	\
				+( .00102830861497023219 +.00003260214424386520*paw8 +.00000024456561422485*paw16 +.00000000013440626754*paw24 +.00000000000001119589*paw32 +.00000000000000000011*paw40)* paw6	\
				+(-.00076578610717556442 +.00000000000000000004*paw40 )* paw8 																													\
				-.00002557906251794953 * paw16																																					\
				-.00000002391582476734 * paw24																																					\
				+.00000000000351377004 * paw32																																					\
				+.00000000000000105160 * paw40 )

double Z(double t, int n)
//*************************************************************************
// Riemann-Siegel Z(t) function implemented per the Riemenn Siegel formula.
// See http://mathworld.wolfram.com/Riemann-SiegelFormula.html for details
//*************************************************************************
{
	double p; /* fractional part of sqrt(t/(2.0*pi))*/
	// double C(int,double); /* coefficient of (2*pi/t)^(k*0.5) */  inutile ?
	constexpr double pi = 3.1415926535897932385; 
	constexpr double two_pi = 2.0 * pi; // precompute 2.0 * pi
	double temp = sqrt(t/(two_pi));
	int N = (int)temp; 
		p = temp - N; 
	double tt = theta(t); 
	volatile double ZZ = 0.0; 
	for (int j=1;j <= N;j++) {
		// 1/sqrt remplacé par rsqrt ?
		ZZ = ZZ + invert_sqrt[j] * cos(tt - t*log_int[j]);
	} 
	ZZ = 2.0 * ZZ; 
	volatile double R  = 0.0; 

	/*
	for (int k=0;k <= n;k++) {
		R = R + C(k,2.0*p-1.0) * pow(2.0*PI/t, ((double) k)*0.5);
	} 
	R = even(N-1) * pow(two_pi / t,0.25) * R; 
	*/
	// Unroll the loop
	const double pow_0 = 1;
	const double two_pi_div_t = two_pi/t;
	const double pow_half = pow(two_pi_div_t, 0.5);
	const double tmp = 2.0*p-1.0;

	// Using #defined version of C() function
	paws(tmp);	

	R += C0 * pow_0;
	R += C1 * pow_half;
	R += C2 * two_pi_div_t;
	R += C3 * (two_pi_div_t) * pow_half;
	R += C4 * two_pi_div_t * two_pi_div_t;

	R = even(N-1) * pow(two_pi_div_t,0.25) * R; 
	return(ZZ + R);
}

/*
	Code to compute Zeta(t) with high precision
	Only works in IEEE 754 for t<1000
	This can help to validate that the Riemann Siegel function for small values but since we are mainly interrested to the behavior for large values of t,
	the best method is to compute zeros that are known
	As you may observe, the accuracy of Z(t) gets better with large values of t until being limited by the IEEE 754 norm and the double format. 
*/
std::complex <double> test_zerod(const double zero,const int N)
{
        std::complex <double> un(1.0,0);  
        std::complex <double> deux(2.0,0); 
        std::complex <double> c1(0.5,zero);
        std::complex <double> sum1(0.0,0.0);
        std::complex <double> sum2(0.0,0.0);
        std::complex <double> p1=un/(un-pow(deux,un-c1));

        for(int k=1;k<=N;k++){
                 std::complex <double> p2=un/pow(k,c1);
                 if(k%2==0)sum1+=p2;
                 if(k%2==1)sum1-=p2;
        }
        std::vector<double   > V1(N);
        std::vector<double   > V2(N);
        double coef=1.0;
        double up=N;
        double dw=1.0;
        double su=0.0;
        for(int k=0;k<N;k++){
                coef*=up;up-=1.0;
                coef/=dw;dw+=1.0;
                V1[k]=coef;
                su+=coef;
        }
        for(int k=0;k<N;k++){
                V2[k]=su;
                su-=V1[k];
        }
        for(int k=N+1;k<=2*N;k++){
                 std::complex <double> p2=un/pow(k,c1);
                 double ek=V2[k-N-1];
                 std::complex <double> c3(ek,0.0);
                 std::complex <double> c4=p2*c3;
                 if(k%2==0)sum2+=c4;
                 if(k%2==1)sum2-=c4;
        }

        std::complex <double> rez=(sum1+sum2/pow(deux,N))*p1;
        return(rez);
}

void test_one_zero(double t)
{
	double RS=Z(t,4);
	std::complex <double> c1=test_zerod(t,10);
	std::complex <double> c2=test_zerod(t,100);
	std::complex <double> c3=test_zerod(t,1000);
	std::cout << std::setprecision(15);
        std::cout << "RS= "<<" "<<RS<<" TEST10= "<< c1 << " TEST100=" << c2 << " TEST1000=" << c3 << std::endl;
	
}

void tests_zeros()
{
	test_one_zero(14.1347251417346937904572519835625);
        test_one_zero(101.3178510057313912287854479402924);
        test_one_zero(1001.3494826377827371221033096531063);
        test_one_zero(10000.0653454145353147502287213889928);

}

/*
	An option to better the performance of Z(t) for large values of t is to simplify the equations
	to validate we present a function that tests the known zeros :  look at https://www.lmfdb.org/zeros/zeta/?limit=10&N=10
	We should obtain 0.0
        no need to test many zeros. In case of a bug the column 2 will show large values instead of values close to 0 like with the original code
	Observe that when t increases the accuracy increases until the limits of the IEEE 754 norm block us, we should work with extended precision
	But here a few digits of precision are enough to count the zeros, only on rare cases the _float128 should be used
	But this limitation only appears very far and with the constraint of resources it won't be possible to reach this region. 
	----------------------------------------------------------------------------------------------------------------------
	value in double			should be 0.0		 value in strings: LMFDB all the digits are corrects
        14.13472514173469463117        -0.00000248590756340983   14.1347251417346937904572519835625
        21.02203963877155601381        -0.00000294582959536882   21.0220396387715549926284795938969
        25.01085758014568938279        -0.00000174024500421144   25.0108575801456887632137909925628
       178.37740777609997167019         0.00000000389177887139   178.3774077760999772858309354141843
       179.91648402025700193008         0.00000000315651035865   179.9164840202569961393400366120511
       182.20707848436646258961         0.00000000214091858131   182.207078484366461915407037226988
 371870901.89642333984375000000         0.00000060389888876036   371870901.8964233245801283081720385309201
 371870902.28132432699203491211        -0.00000083698274928878   371870902.2813243157291041227177012243450
 371870902.52132433652877807617        -0.00000046459056067712   371870902.5213243412580878836297930128983
*/

char line[1024];
void test_fileof_zeros(const char *fname)
{
	FILE *fi=fopen(fname,"r");
	assert(fi!=NULL);
	for(;;){
		double t,RS;
		fgets(line,1000,fi);
		if(feof(fi))break;
		sscanf(line,"%lf",&t);
		RS=Z(t,4);
		printf(" %30.20lf %30.20lf   %s",t,RS,line);

	}
	fclose(fi);
}

int main(int argc,char **argv) 
{
	double LOWER,UPPER,SAMP;
	const double pi = 3.1415926535897932385;
	//tests_zeros();
	//test_fileof_zeros("ZEROS");
	try {
		LOWER=std::atof(argv[1]);
		UPPER=std::atof(argv[2]);
		SAMP =std::atof(argv[3]);
	}
	catch (...) {
				std::cout << argv[0] << " START END SAMPLING" << std::endl;
				return -1;
	}
	double estimate_zeros=theta(UPPER)/pi;
	printf("I estimate I will find %1.3lf zeros\n",estimate_zeros);
	double STEP = 1.0/SAMP;
	ui64   NUMSAMPLES=floor((UPPER-LOWER)*SAMP+1.0);
	double prev=0.0;
	double count=0.0;
	double t1 = dml_micros();
	
	compute_table(sqrt(UPPER/(2*pi))+1);

	#pragma omp parallel firstprivate(prev) reduction(+:count) shared(invert_sqrt,log_int) 
	{
		const ui32 nb_thread = omp_get_num_threads();
		const ui32 th_id = omp_get_thread_num();
		ui64 TASK_STEP = floor(NUMSAMPLES/nb_thread);
		ui64 THREAD_STEP = floor(TASK_STEP/nb_thread);
		ui64 task_i = 0;
		ui64 TASK_LOWER = 0;
		ui64 TASK_UPPER = 0;
		ui64 THREAD_LOWER = 0;
		ui64 THREAD_UPPER = 0;
		for(task_i = 0 ; task_i <  nb_thread ; task_i++)
		{
			TASK_LOWER = (double)task_i * TASK_STEP;
			TASK_UPPER = (double)(task_i + 1) * TASK_STEP;
			THREAD_LOWER = (double)th_id * THREAD_STEP + TASK_LOWER;
			THREAD_UPPER = (double)(th_id + 1) * THREAD_STEP + TASK_LOWER;
			prev = Z(THREAD_LOWER*STEP + LOWER, 4);
			volatile ui64 t = 0.0;
			for (t = THREAD_LOWER; t <= THREAD_UPPER; t++)
			{
				//printf("%d %f\n",t ,LOWER+STEP*t);
				double zout=Z(STEP*t + LOWER,4);
				count += (signbit(zout) != signbit(prev));
				prev=zout;
			}
		}
		
		TASK_STEP = NUMSAMPLES - TASK_UPPER;
		volatile ui64 t = 0.0;
		//printf("Las thread num sample %d \n", NUMSAMPLES - TASK_UPPER);
		prev = Z(THREAD_UPPER*STEP + LOWER,4);
		if(th_id == nb_thread - 1)
		{
			for ( t = THREAD_UPPER; t <= NUMSAMPLES; t++)
			{
				double zout=Z(LOWER+STEP*t,4);
				//printf("%d %f\n",t ,LOWER+STEP*t);
				count += (signbit(zout) != signbit(prev));
				prev=zout;
			}
		}
		
	}

	double t2=dml_micros();

	printf("I found %1.0lf Zeros in %.3lf seconds\n",count,(t2-t1)/1000000.0);
	log_int.clear();
	invert_sqrt.clear();
	return(0);
}
