#include "Arithmpz.h"
#include <vector>

using namespace std;	

namespace model{
	mpz_class operator+(const mpz_class a,const mpz_class b){
		mpz_t res;
		mpz_init(res);
		
		mpz_add (res, a.get_mpz_t(), b.get_mpz_t());
		
		//mpz_class p(PRIME);
		
		//mpz_mod (res, res, p.get_mpz_t());
		return mpz_class(res);
	}
	
	mpz_class operator-(const mpz_class a,const mpz_class b){
		mpz_t res;
		mpz_init(res);
		
		mpz_sub (res, a.get_mpz_t(), b.get_mpz_t());
		
		//mpz_class p(PRIME);
		
		//mpz_mod (res, res, p.get_mpz_t());
		return mpz_class(res);
	}
	
	mpz_class operator*(const mpz_class a,const mpz_class b){
		mpz_t res;
		mpz_init(res);
		
		mpz_mul (res, a.get_mpz_t(), b.get_mpz_t());
		
		//mpz_class p(PRIME);
		
		//mpz_mod (res, res, p.get_mpz_t());
		return mpz_class(res);
	}
	
	mpz_class operator/(const mpz_class a,const mpz_class b){
		mpz_t res;
		mpz_init(res);
		
		mpz_cdiv_q (res, a.get_mpz_t(), b.get_mpz_t());
		
		//mpz_class p(PRIME);
		
		//mpz_mod (res, res, p.get_mpz_t());
		return mpz_class(res);
	}
	
	mpz_class operator%(const mpz_class a,const mpz_class b){
		mpz_t res;
		mpz_init(res);
		
		mpz_mod (res, a.get_mpz_t(), b.get_mpz_t());
		return mpz_class(res);
	}
	
	vector<mpz_class> add(const vector<mpz_class> a,const vector<mpz_class> b){
		vector<mpz_class> res(std::max(a.size(), b.size()));
		for(int i = 0; i < a.size(); ++i)
			res[i] = a[i];
		for(int i = 0; i < b.size(); ++i)
			res[i] = b[i];
		return res;
	}
	
	vector<mpz_class> mult(const vector<mpz_class> a,const vector<mpz_class> b){
		cout << "on a A = " << endl;
			for(int l = 0; l < a.size(); ++l)
				cout << a[l] << endl;
		cout << "et B = " << endl;
			for(int l = 0; l < b.size(); ++l)
				cout << b[l] << endl;
		if(a.size() == 1){
			mult(b, a[0]);
			return b;
		}else if(b.size() == 1){
			mult(a, b[0]);
			return a;
		}
		int cap1 = a.size(), cap2 = b.size();
		cout << "cap1 = " << cap1 << endl;
		cout << "cap2 = " << cap2 << endl;
		while(cap1 > 0 && a[cap1-1] == 0) 
			--cap1;
		while(cap2 > 0 && b[cap2-1] == 0) 
			--cap2;
		cout << "cap1 after = " << cap1 << endl;
		cout << "cap2 after = " << cap2 << endl;
		vector<mpz_class> res(cap1 + cap2 - 1);
		for(int i = 0; i < cap1; ++i){
			for(int j = 0; j < cap2; ++j){
				res[i+j] += a[i]*b[j];
			}
		}
		
		cout << "en sortant de la mult, on a res = " << endl;
			for(int l = 0; l < res.size(); ++l)
				cout << res[l] << endl;
		
		return res;
	}
	
	void mult(vector<mpz_class> a, const mpz_class b){
		cout << "on a A = " << endl;
			for(int l = 0; l < a.size(); ++l)
				cout << a[l] << endl;
		
		for(int i = 0; i < a.size(); ++i)
			a[i] *= b;
		
	}
	
	vector<mpz_class> sub(const vector<mpz_class> a,const vector<mpz_class> b,const vector<mpz_class> c){
		vector<mpz_class> res(a.size());
		cout << "res.size() = " << res.size() << endl;
		for(int i = 0; i < a.size(); ++i){
			res[i] = a[i];
			if(i < b.size())
				res[i] = res[i] - b[i];
			if(i < c.size())
				res[i] = res[i] - c[i];
			cout << "i pb = " << i << ", res[i] = " << res[i] << endl;
		}
		
		return res;
	}
}


