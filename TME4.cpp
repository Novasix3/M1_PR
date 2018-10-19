#include <iostream>

#include <gmpxx.h> //On inclut la bibliothèque gmp
#include <chrono>
#include <vector>
#include "Arithmpz.h"

#define MPZ mpz_class

using namespace std;
using namespace model;

vector<vector<MPZ>> Strassen(vector<vector<MPZ>> P, vector<vector<MPZ>> Q){
	if(P.size() == 2){
		MPZ M1 = (P[0][0] + P[1][1]) * (Q[0][0] + Q[1][1]);
		MPZ M2 = (P[1][0] + P[1][1]) * (Q[0][0]);
		MPZ M3 = (P[0][0]) * (Q[0][1] - Q[1][1]);
		MPZ M4 = (P[1][1]) * (Q[1][0] - Q[0][0]);
		MPZ M5 = (P[0][0] + P[0][1]) * (Q[1][1]);
		MPZ M6 = (P[1][0] - P[0][0]) * (Q[0][0] + Q[0][1]);
		MPZ M7 = (P[0][1] - P[1][1]) * (Q[1][0] + Q[1][1]);
		
		vector<MPZ> C1(2);
		C1[0] = M1+M4-M5+M7;
		C1[1] = M3+M5;
		vector<MPZ> C2(2);
		C2[0] = M2+M4;
		C2[1] = M1-M2+M3+M6;
		
		vector<vector<MPZ>> C(2);
		C[0] = C1;
		C[1] = C2;
		
		return C;
	}
	vector<vector<MPZ>>
	
	return P;
}

vector<vector<MPZ>> naiveMultiply(vector<vector<MPZ>> P, vector<vector<MPZ>> Q){
	if(P.size() != Q[0].size()){
		cout << "impossible to multiply : size of matrices invalid" << endl;
		exit(0);
	}
	vector<vector<MPZ>> res(P.size());
	for(int i = 0; i < P.size(); ++i){
		cout << "i = " << i << endl;
		vector<MPZ> temp(Q[0].size());
		for(int l = 0; l < Q[0].size(); ++l){
			MPZ total(0);
			cout << "	l = " << l << endl;
			for(int j = 0; j < Q.size(); ++j){
				cout << "		j = " << j << endl;
				cout << "		P[i][j] = " << P[i][j] << endl;
				cout << "		Q[j][l] = " << Q[j][l] << endl;
				total += P[i][j] * Q[j][l];
			}
			temp[l] = total;
		}
		cout << "[";
		for(auto a: temp)
			cout << a << ",";
		cout << "]" << endl;
		res[i] = temp;
	}
	return res;
}

int main(int argc, char **argv)

{ 
	
	vector<MPZ> A(4);
	A[0] = 0;
	A[1] = 1;
	A[2] = 2;
	A[3] = 3;
	
	vector<MPZ> B(4);
	B[0] = 4;
	B[1] = 5;
	B[2] = 6;
	B[3] = 7;
	
	vector<MPZ> C(4);
	C[0] = 8;
	C[1] = 9;
	C[2] = 10;
	C[3] = 11;
	
	vector<MPZ> D(4);
	D[0] = 12;
	D[1] = 13;
	D[2] = 14;
	D[3] = 15;
	
	vector<vector<MPZ>> P(4);
	P[0] = A;
	P[1] = B;
	P[2] = C;
	P[3] = D;
	
	vector<MPZ> E(4);
	E[0] = 16+0;
	E[1] = 16+1;
	E[2] = 16+2;
	E[3] = 16+3;
	
	vector<MPZ> F(4);
	F[0] = 16+4;
	F[1] = 16+5;
	F[2] = 16+6;
	F[3] = 16+7;

	vector<MPZ> G(4);
	G[0] = 16+8;
	G[1] = 16+9;
	G[2] = 16+10;
	G[3] = 16+11;
	
	vector<MPZ> H(4);
	H[0] = 16+12;
	H[1] = 16+13;
	H[2] = 16+14;
	H[3] = 16+15;
		
	vector<vector<MPZ>> Q(4);
	Q[0] = E;
	Q[1] = F;
	Q[2] = G;
	Q[3] = H;
	
	vector<vector<MPZ>> res(naiveMultiply(P,Q));
	cout << endl << endl;
	for(auto a: res){
		cout << "[";
		for(auto b:a){
			cout << b << ",";
		}
		cout << "]" << endl;
	}

    return 0;

}
