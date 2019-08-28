#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>

double my_gettimeofday(){
  struct timeval tmp_time;
  gettimeofday(&tmp_time, NULL);
  return tmp_time.tv_sec + (tmp_time.tv_usec * 1.0e-6L);
}

///////////////////////////////////////////////////////////////////////
/// WARNING: naive algorithm with worst operation count!
unsigned long long fib(unsigned long long n, int prof_max) {
  //printf("appel avec %d\n", n);
  unsigned long long i, j;
  if (n<2)
    return n;
  else if(prof_max <= 0){
    i=fib(n-1, 0);
    j=fib(n-2, 0);
    return i+j;
  }else {
    #pragma omp task shared(i)
    i=fib(n-1, prof_max - 1);

    #pragma omp task shared(j)
    j=fib(n-2, prof_max - 1);
    
    #pragma omp taskwait
    return i+j;
  }
}


///////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
  double debut=0.0, fin=0.0;
  unsigned long long n=50; /* default value -> roughly 10 seconds of computation */
  int res=0;

  /* Read 'n' on command line: */
  if (argc == 2){
    n = atoi(argv[1]);
  }

  /* Start timing */
  debut = my_gettimeofday();

  /* Do computation:  */
  #pragma omp parallel
  {
    #pragma omp single
    res=fib(n, 20);
  }
     
  /* End timing */
  fin = my_gettimeofday();
  fprintf(stdout, " fib(%lld)=%d\n", n, res);
  fprintf( stdout, "For n=%lld: total computation time (with gettimeofday()) : %g s\n",
	   n, fin - debut);
      
  return 0;
}



