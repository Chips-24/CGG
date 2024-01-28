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

inline __attribute__((always_inline)) int even(int n)
{
	return 1-2*(n&1);
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

#ifndef C_PROG
struct paw_t {
	double z2;
	double z4;
	double z6;
	double z8;
	double z16;
	double z24;
	double z32;
	double z40;
	double z1;
	double z3;
	double z5;
	double z7;
	double z9;
	double z17;
	double z25;
	double z33;
	double z41;


	inline paw_t(const double &z) 
	{
		z2 	= z*z;
		z4 	= z2 *z2;
		z6 	= z4 *z2;
		z8 	= z4 *z4;
		z16	= z8 *z8;
		z24	= z16*z8;
		z32	= z16*z16;
		z40	= z32*z8;
		z1 	= z;
		z3 	= z2*z;
		z5 	= z4*z;
		z7 	= z6*z;
		z9 	= z8*z;
		z17	= z16*z;
		z25	= z24*z;
		z33	= z32*z;
		z41	= z40*z;
	}

};

inline double C0(const paw_t &paw) {
	return(	.38268343236508977173
			+( .43724046807752044936 -.00162372532314446528*paw.z8 -.00000143272516309551*paw.z16 -.00000000003391414390*paw.z24 +.00000000000000522184*paw.z32 +.00000000000000000015*paw.z40)* paw.z2
			+( .13237657548034352332 +.00029705353733379691*paw.z8 -.00000010354847112313*paw.z16 -.00000000001632663390*paw.z24 -.00000000000000033507*paw.z32)* paw.z4
			+(-.01360502604767418865 +.00007943300879521470*paw.z8 +.00000001235792708386*paw.z16 -.00000000000037851093*paw.z24 -.00000000000000003412*paw.z32)* paw.z6
			-.01356762197010358089 * paw.z8
			+.00000046556124614505 * paw.z16
			+.00000000178810838580 * paw.z24
			+.00000000000009327423 * paw.z32
			+.00000000000000000058 * paw.z40 );
}

inline double C1(const paw_t &paw) 
{
	return(	-.02682510262837534703 * paw.z1
			+.01378477342635185305 * paw.z3
			+.03849125048223508223 * paw.z5
			+.00987106629906207647 * paw.z7
			-.00331075976085840433 * paw.z9
			+(-.00146478085779541508*paw.z9 -.00000096413224561698*paw.z17 +.00000000007785288654*paw.z25 +.00000000000001458378*paw.z33 +.00000000000000000036*paw.z41)* paw.z2
			+(-.00001320794062487696*paw.z9 -.00000018334733722714*paw.z17 -.00000000002343762601*paw.z25 -.00000000000000028786*paw.z33 +.00000000000000000001*paw.z41)* paw.z4
			+( .00005922748701847141*paw.z9 +.00000000446708756272*paw.z17 -.00000000000158301728*paw.z25 -.00000000000000008663*paw.z33)* paw.z6
			+.00000598024258537345 * paw.z17
			+.00000000270963508218 * paw.z25
			+.00000000000012119942 * paw.z33
			-.00000000000000000084 * paw.z41 );
}

inline double C2(const paw_t &paw)
{
	return(	+.00518854283029316849
			+( .00030946583880634746 +.00034399144076208337*paw.z8 +.00000592766549309654*paw.z16 +.00000000209115148595*paw.z24 +.00000000000005398265*paw.z32 -.00000000000000000416*paw.z40)* paw.z2
			+(-.01133594107822937338 -.00059106484274705828*paw.z8 -.00000016423838362436*paw.z16 +.00000000017815649583*paw.z24 +.00000000000001975014*paw.z32 +.00000000000000000044*paw.z40)* paw.z4
			+( .00223304574195814477 -.00010229972547935857*paw.z8 -.00000015161199700941*paw.z16 -.00000000001616407246*paw.z24 +.00000000000000023333*paw.z32 +.00000000000000000003*paw.z40)* paw.z6
			+.00519663740886233021 * paw.z8
			+.00002088839221699276 * paw.z16
			-.00000000590780369821 * paw.z24
			-.00000000000238069625 * paw.z32
			-.00000000000000011188 * paw.z40 );
}

inline double C3(const paw_t &paw)
{
	return(	-.00133971609071945690 * paw.z1
			+.00374421513637939370 * paw.z3
			-.00133031789193214681 * paw.z5
			-.00226546607654717871 * paw.z7
			+.00095484999985067304 * paw.z9
			+( .00060100384589636039*paw.z9 +.00000333165985123995*paw.z17 +.00000000095701162109*paw.z25 -.00000000000003627687*paw.z33 -.00000000000000000713*paw.z41)* paw.z2
			+(-.00010128858286776622*paw.z9 +.00000021919289102435*paw.z17 +.00000000018763137453*paw.z25 +.00000000000001763981*paw.z33 +.00000000000000000033*paw.z41)* paw.z4
			+(-.00006865733449299826*paw.z9 -.00000007890884245681*paw.z17 -.00000000000443783768*paw.z25 +.00000000000000079608*paw.z33 +.00000000000000000004*paw.z41)* paw.z6
			+.00000059853667915386 * paw.z17
			-.00000000941468508130 * paw.z25
			-.00000000000224267385 * paw.z33
			-.00000000000000009420 * paw.z41 );
}

inline double C4(const paw_t &paw)
{
	return(	+.00046483389361763382
			+(-.00100566073653404708 -.00020365286803084818*paw.z8 -.00000410746443891574*paw.z16 -.00000000750521420704*paw.z24 -.00000000000151915445*paw.z32 -.00000000000000005179*paw.z40)* paw.z2
			+( .00024044856573725793 +.00023212290491068728*paw.z8 +.00000117811136403713*paw.z16 +.00000000013312279416*paw.z24 -.00000000000008915418*paw.z32 -.00000000000000000807*paw.z40)* paw.z4
			+( .00102830861497023219 +.00003260214424386520*paw.z8 +.00000024456561422485*paw.z16 +.00000000013440626754*paw.z24 +.00000000000001119589*paw.z32 +.00000000000000000011*paw.z40)* paw.z6
			+(-.00076578610717556442 +.00000000000000000004*paw.z40 )* paw.z8
			-.00002557906251794953 * paw.z16	
			-.00000002391582476734 * paw.z24	
			+.00000000000351377004 * paw.z32	
			+.00000000000000105160 * paw.z40 );
}

#else
typedef struct paw_s {
	double z2;
	double z4;
	double z6;
	double z8;
	double z16;
	double z24;
	double z32;
	double z40;
	double z1;
	double z3;
	double z5;
	double z7;
	double z9;
	double z17;
	double z25;
	double z33;
	double z41;
} paw_t;

inline __attribute__((always_inline)) void paw_init(paw_t *paw, const double z) 
{
	paw->z2 	= z*z;
	paw->z4 	= paw->z2 *paw->z2;
	paw->z6 	= paw->z4 *paw->z2;
	paw->z8 	= paw->z4 *paw->z4;
	paw->z16	= paw->z8 *paw->z8;
	paw->z24	= paw->z16*paw->z8;
	paw->z32	= paw->z16*paw->z16;
	paw->z40	= paw->z32*paw->z8;
	paw->z1 	= z;
	paw->z3 	= paw->z2*z;
	paw->z5 	= paw->z4*z;
	paw->z7 	= paw->z6*z;
	paw->z9 	= paw->z8*z;
	paw->z17	= paw->z16*z;
	paw->z25	= paw->z24*z;
	paw->z33	= paw->z32*z;
	paw->z41	= paw->z40*z;
}

inline double C0(const paw_t *paw) {
	return(	.38268343236508977173
			+( .43724046807752044936 -.00162372532314446528*paw->z8 -.00000143272516309551*paw->z16 -.00000000003391414390*paw->z24 +.00000000000000522184*paw->z32 +.00000000000000000015*paw->z40)* paw->z2
			+( .13237657548034352332 +.00029705353733379691*paw->z8 -.00000010354847112313*paw->z16 -.00000000001632663390*paw->z24 -.00000000000000033507*paw->z32)* paw->z4
			+(-.01360502604767418865 +.00007943300879521470*paw->z8 +.00000001235792708386*paw->z16 -.00000000000037851093*paw->z24 -.00000000000000003412*paw->z32)* paw->z6
			-.01356762197010358089 * paw->z8
			+.00000046556124614505 * paw->z16
			+.00000000178810838580 * paw->z24
			+.00000000000009327423 * paw->z32
			+.00000000000000000058 * paw->z40 );
}

inline double C1(const paw_t *paw) 
{
	return(	-.02682510262837534703 * paw->z1
			+.01378477342635185305 * paw->z3
			+.03849125048223508223 * paw->z5
			+.00987106629906207647 * paw->z7
			-.00331075976085840433 * paw->z9
			+(-.00146478085779541508*paw->z9 -.00000096413224561698*paw->z17 +.00000000007785288654*paw->z25 +.00000000000001458378*paw->z33 +.00000000000000000036*paw->z41)* paw->z2
			+(-.00001320794062487696*paw->z9 -.00000018334733722714*paw->z17 -.00000000002343762601*paw->z25 -.00000000000000028786*paw->z33 +.00000000000000000001*paw->z41)* paw->z4
			+( .00005922748701847141*paw->z9 +.00000000446708756272*paw->z17 -.00000000000158301728*paw->z25 -.00000000000000008663*paw->z33)* paw->z6
			+.00000598024258537345 * paw->z17
			+.00000000270963508218 * paw->z25
			+.00000000000012119942 * paw->z33
			-.00000000000000000084 * paw->z41 );
}

inline double C2(const paw_t *paw)
{
	return(	+.00518854283029316849
			+( .00030946583880634746 +.00034399144076208337*paw->z8 +.00000592766549309654*paw->z16 +.00000000209115148595*paw->z24 +.00000000000005398265*paw->z32 -.00000000000000000416*paw->z40)* paw->z2
			+(-.01133594107822937338 -.00059106484274705828*paw->z8 -.00000016423838362436*paw->z16 +.00000000017815649583*paw->z24 +.00000000000001975014*paw->z32 +.00000000000000000044*paw->z40)* paw->z4
			+( .00223304574195814477 -.00010229972547935857*paw->z8 -.00000015161199700941*paw->z16 -.00000000001616407246*paw->z24 +.00000000000000023333*paw->z32 +.00000000000000000003*paw->z40)* paw->z6
			+.00519663740886233021 * paw->z8
			+.00002088839221699276 * paw->z16
			-.00000000590780369821 * paw->z24
			-.00000000000238069625 * paw->z32
			-.00000000000000011188 * paw->z40 );
}

inline double C3(const paw_t *paw)
{
	return(	-.00133971609071945690 * paw->z1
			+.00374421513637939370 * paw->z3
			-.00133031789193214681 * paw->z5
			-.00226546607654717871 * paw->z7
			+.00095484999985067304 * paw->z9
			+( .00060100384589636039*paw->z9 +.00000333165985123995*paw->z17 +.00000000095701162109*paw->z25 -.00000000000003627687*paw->z33 -.00000000000000000713*paw->z41)* paw->z2
			+(-.00010128858286776622*paw->z9 +.00000021919289102435*paw->z17 +.00000000018763137453*paw->z25 +.00000000000001763981*paw->z33 +.00000000000000000033*paw->z41)* paw->z4
			+(-.00006865733449299826*paw->z9 -.00000007890884245681*paw->z17 -.00000000000443783768*paw->z25 +.00000000000000079608*paw->z33 +.00000000000000000004*paw->z41)* paw->z6
			+.00000059853667915386 * paw->z17
			-.00000000941468508130 * paw->z25
			-.00000000000224267385 * paw->z33
			-.00000000000000009420 * paw->z41 );
}

inline double C4(const paw_t *paw)
{
	return(	+.00046483389361763382
			+(-.00100566073653404708 -.00020365286803084818*paw->z8 -.00000410746443891574*paw->z16 -.00000000750521420704*paw->z24 -.00000000000151915445*paw->z32 -.00000000000000005179*paw->z40)* paw->z2
			+( .00024044856573725793 +.00023212290491068728*paw->z8 +.00000117811136403713*paw->z16 +.00000000013312279416*paw->z24 -.00000000000008915418*paw->z32 -.00000000000000000807*paw->z40)* paw->z4
			+( .00102830861497023219 +.00003260214424386520*paw->z8 +.00000024456561422485*paw->z16 +.00000000013440626754*paw->z24 +.00000000000001119589*paw->z32 +.00000000000000000011*paw->z40)* paw->z6
			+(-.00076578610717556442 +.00000000000000000004*paw->z40 )* paw->z8
			-.00002557906251794953 * paw->z16	
			-.00000002391582476734 * paw->z24	
			+.00000000000351377004 * paw->z32	
			+.00000000000000105160 * paw->z40 );
}
#endif


double Z(double t, int n)
//*************************************************************************
// Riemann-Siegel Z(t) function implemented per the Riemenn Siegel formula.
// See http://mathworld.wolfram.com/Riemann-SiegelFormula.html for details
//*************************************************************************
{
	double p; /* fractional part of sqrt(t/(2.0*pi))*/
	double C(int,double); /* coefficient of (2*pi/t)^(k*0.5) */
	const double pi = 3.1415926535897932385; 
	const double two_pi = 2.0 * pi;
	double tmp = sqrt(t/two_pi); 
	int N = (int)tmp;
		p = tmp - (double)N;
	double tt = theta(t); 
	double ZZ = 0.0; 
	for (int j=1;j <= N;j++) {
		ZZ = ZZ + 1.0/sqrt((double) j ) * cos(fmod(tt -t*log((double) j),2.0*pi));
	} 
	ZZ = 2.0 * ZZ; 
	double R  = 0.0; 
	// for (int k=0;k <= n;k++) {
	// 	R = R + C(k,2.0*p-1.0) * pow(2.0*pi/t, ((double) k)*0.5);
	// } 
	// R = even(N-1) * pow(2.0 * pi / t,0.25) * R;

	// Unrolled loop
	const double pow_0 = 1;
	const double two_pi_over_t = two_pi/t;
	const double pow_half = pow(two_pi_over_t,0.5);
	const double temp = 2.0*p-1.0;
#ifndef C_PROG
	paw_t paw(temp);
#else
	paw_t paw_v;
	paw_t *paw=&paw_v;
	paw_init(paw,temp);
#endif

	R += C0(paw) * pow_0;
	R += C1(paw) * pow_half;
	R += C2(paw) * two_pi_over_t;
	R += C3(paw) * pow_half * two_pi_over_t;
	R += C4(paw) * two_pi_over_t * two_pi_over_t;

	R = even(N-1) * pow(two_pi_over_t,0.25) * R;

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
