#include <stdlib.h>
#include <stdio.h>

#include "Exo1.h"

void madd(long* res, long a, long b, long P){
	*res = (a+b)%P;
}

void msub(long* res, long a, long b, long P){
	b *= -1;
	while(b < 0)
		b += P;
	madd(res, a, b, P);
}

void mmult(long* res, long a, long b, long P){
	*res = (a*b)%P;
}

void euclide(long* u, long* v, long a, long b){
	long u0 = 1, u1 = 0;
	long v0 = 0, v1 = 1;
	long r0 = a, r1 = b;
	long q = 0;

	printf("a = %lu, b = %lu\n", a, b);

	while(r1 != 0){
		q = r0 / r1;

		printf("q = %lu\n", q);

		long ut = u0 - q*u1;
		u0 = u1;
		u1 = ut;

		long vt = v0 - q*v1;
		v0 = v1;
		v1 = vt;

		long rt = r0 - q*r1;
		r0 = r1;
		r1 = rt;

		printf("r0 = %d, r1 = %d\n", r0, r1);
	} 

	printf("u0 = %d, v0 = %d\n", u0, v0);
	*u = u0;
	*v = v0;
}

void invert(long* res, long a, long P){
	long ru = 0, rv = 0;
	euclide(&ru, &rv, a, P);
	*res = (ru%P + P)%P;
}

void mdiv(long* res, long a, long b){

}