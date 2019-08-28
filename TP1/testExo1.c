#include<stdlib.h>
#include<stdio.h>

#include "Exo1.h"

int main(){
	long a = 3, b = 6, c = 7;
	long res = 0;
	madd(&res, a, b, c);
	printf("(a+b) mod 9 = %ld\n", res);
	msub(&res, a, b, c);
	printf("(a-b) mod 9 = %ld\n", res);
	mmult(&res, a, b, c);
	printf("(a*b) mod 9 = %ld\n", res);

	long u = 0, v = 0;
	euclide(&u, &v, a, 5);
	printf("a*%ld + b*%ld = 1\n", u, v);

	invert(&u, a, c);
	printf("a*%ld = 1 mod %ld\n", u, c);
}