#include <sys/time.h>
#include <stdio.h>
#include <immintrin.h>
//#define N 128
#define N 1024
//#define N 16384
//#define N 1048576
//#define N 16777216
//#define N 134217728
#define NB_TIMES 1000000

double my_gettimeofday()
{
	struct timeval tmp_time;
	gettimeofday(&tmp_time, NULL);
	return tmp_time.tv_sec + (tmp_time.tv_usec * 1.0e-6L);
}

float A[N] __attribute__ ((aligned(32)));
float B[N] __attribute__ ((aligned(32)));
float C[N] __attribute__ ((aligned(32)));

int main()
{
	int i, k;
	double start, stop;
	float res = 0.0;

	for (i = 0; i < N; i++) {
		A[i] = 1.0;
		B[i] = 1.0;
		C[i] = 0.0;
	}

	__m256 v1, v2, v3;

	start = my_gettimeofday();

	for (k = 0; k < NB_TIMES; k++) {
		for (i = 0; i < N; i += 8) {
			v1 = _mm256_load_ps(A + i);
			v2 = _mm256_load_ps(B + i);

			v3 = _mm256_mul_ps(v1, v2);

			_mm256_store_ps(C + i, v3);
		}
	}

	stop = my_gettimeofday();
	fprintf(stdout, "C[0]=%f C[1]=%f C[4]=%f C[8]=%f C[N-1]=%f \n",
		C[0], C[1], C[4], C[8], C[N - 1]);
	/*for(int i = 0; i < N; ++i){
		fprintf(stdout, "C[%d]=%f, ", i, C[i]);
	}*/
	fprintf(stdout, "Temps total de calcul : %g sec\n", stop - start);

	return 0;
}
