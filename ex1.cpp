#include <iostream>

#include <gmpxx.h> //On inclut la bibliothèque gmp
#include <chrono>

#define MAX 10 //On affichera MAX nombres premiers...
#define PRIME 7
  
void multiply(mpz_class F[2][2], mpz_class M[2][2]); 
  
void power(mpz_class F[2][2], mpz_class n); 

mpz_class fibo3(mpz_class n) 
{ 
 	mpz_class un(1), zero(0);
  mpz_class F[2][2] = {{un,un},{un,zero}}; 
  if (n == 0) 
    return 0; 
  power(F, n-1); 
  return F[0][0]; 
} 
  

void power(mpz_class F[2][2], mpz_class n) 
{ 
  if( n == 0 || n == 1) 
      return; 
  mpz_class M[2][2] = {{1,1},{1,0}}; 
  
  power(F, n/2); 
  multiply(F, F); 
  
  if (n%2 != 0) 
     multiply(F, M); 
} 
  
void multiply(mpz_class F[2][2], mpz_class M[2][2]) 
{ 
  mpz_class x =  F[0][0]*M[0][0] + F[0][1]*M[1][0]; 
  mpz_class y =  F[0][0]*M[0][1] + F[0][1]*M[1][1]; 
  mpz_class z =  F[1][0]*M[0][0] + F[1][1]*M[1][0]; 
  mpz_class w =  F[1][0]*M[0][1] + F[1][1]*M[1][1]; 
  
  F[0][0] = x; 
  F[0][1] = y; 
  F[1][0] = z; 
  F[1][1] = w; 
} 

mpz_class fibo2(mpz_class n){
	mpz_class a(1), b(2);
	for(mpz_class i = 3; i < n; i++){
		mpz_class temp = a;
		a = b;
		b = temp + b;
	}
	return (a<b)?b:a;
}

void fib(long unsigned int n){
	mpz_t res;
	mpz_init(res);
	mpz_fib_ui(res, n);
    	std::cout << "fibo(7) = " << res << std::endl;
	
}

mpz_class operator+(const mpz_class a,const mpz_class b)
{
	
//std::cout << " a = " << a << std::endl;
	//	std::cout << " b = " << b << std::endl;
	mpz_t res;
	mpz_init(res);
	
	mpz_add (res, a.get_mpz_t(), b.get_mpz_t());
	
	//std::cout << " res = " << res << std::endl;
	
	mpz_class p(PRIME);
	
	mpz_mod (res, res, p.get_mpz_t());
	return mpz_class(res);
}

mpz_class operator+(const mpz_class a,const mpz_class b)
{
	
//std::cout << " a = " << a << std::endl;
	//	std::cout << " b = " << b << std::endl;
	mpz_t res;
	mpz_init(res);
	
	mpz_add (res, a.get_mpz_t(), b.get_mpz_t());
	
	//std::cout << " res = " << res << std::endl;
	
	mpz_class p(PRIME);
	
	mpz_mod (res, res, p.get_mpz_t());
	return mpz_class(res);
}


int main(int argc, char **argv)

{


/*auto start = std::chrono::high_resolution_clock::now();
	//fib(907877777);
	
auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
	std::cout << "Elapsed time: " << elapsed.count() << " s\n";

mpz_class a(907877777);



	start = std::chrono::high_resolution_clock::now();
    //std::cout << "fibo(7) = " << fibo2(a) << std::endl;
    std::cout << "fibo(7) = " << fibo3(9078) << std::endl;
	finish = std::chrono::high_resolution_clock::now();
	elapsed = finish - start;
	std::cout << "Elapsed time: " << elapsed.count() << " s\n";*/
	mpz_class a(4), b(9);
	
    std::cout << "fibo(7) = " << a+b << std::endl;


    return 0;

}
