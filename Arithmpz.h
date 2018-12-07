#ifndef ARITHMPZ_H_
#define ARITHMPZ_H_

#include <cstdlib>
#include <iostream>
#include <vector>

#include <gmpxx.h>

#define PRIME 7

using namespace std;

namespace model{
			vector<mpz_class> add(const vector<mpz_class> a,const vector<mpz_class> b);
			vector<mpz_class> mult(const vector<mpz_class> a,const vector<mpz_class> b);
			void mult(vector<mpz_class> a, const mpz_class b);
			vector<mpz_class> sub(const vector<mpz_class> a,const vector<mpz_class> b,const vector<mpz_class> c);
	class Arithmpz{
		public:
			friend mpz_class operator+(const mpz_class a,const mpz_class b);
			friend mpz_class operator*(const mpz_class a,const mpz_class b);
			friend mpz_class operator/(const mpz_class a,const mpz_class b);
			friend mpz_class operator%(const mpz_class a,const mpz_class b);
	};
}



#endif
