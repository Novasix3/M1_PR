#include <iostream>

#include <gmpxx.h> //On inclut la bibliothèque gmp
#include <chrono>
#include <vector>
#include "Arithmpz.h"
#include <math.h>
#include <typeinfo>

#define MPZ mpz_class

using namespace std;
using namespace model;

vector<MPZ> Euclid(MPZ A, MPZ B){
	vector<MPZ> R(2);
	R[0] = A;
	R[1] = B;
	vector<MPZ> U(2);
	U[0] = 1;
	U[1] = 0;
	vector<MPZ> V(2);
	V[0] = 0;
	V[1] = 1;
	while(R[1] != 0){
		cout << "\n\n\n" << endl;
		cout << "R[0] = " << R[0] << endl;
		cout << "R[1] = " << R[1] << endl;
		cout << "U[0] = " << U[0] << endl;
		cout << "U[1] = " << U[1] << endl;
		cout << "V[0] = " << V[0] << endl;
		cout << "V[1] = " << V[1] << endl;
		cout << R[0] / R[1] << endl ;
		MPZ q(R[0]/R[1]);
		vector<MPZ> temp(3);
		temp[0] = R[0] - q*R[1];
		temp[1] = U[0] - q*U[1];
		temp[2] = V[0] - q*V[1];
		cout << "temp[0] = " << temp[0] << endl;
		cout << "temp[1] = " << temp[1] << endl;
		cout << "temp[2] = " << temp[2] << endl;
		R[0] = R[1];
		R[1] = temp[0];
		U[0] = U[1];
		U[1] = temp[1];
		V[0] = V[1];
		V[1] = temp[2];
		cout << "after R[0] = " << R[0] << endl;
		cout << "after R[1] = " << R[1] << endl;
	}
	vector<MPZ> res(2);
	res[0] = U[0];
	res[1] = V[0];
	return res;
}

MPZ CRT(vector<MPZ> A, vector<MPZ> B){
	vector<MPZ> Bezout = Euclid(B[0], B[1]);
	cout << Bezout[0] << ", " << Bezout[1] << endl;
	cout << A[0] << ", " << A[1] << endl;
	cout << B[0] << ", " << B[1] << endl;
	return (B[0] * B[1] + A[0] * B[1] * Bezout[1] + A[1] * B[0] * Bezout[0])%(B[0] * B[1]);
}

MPZ CRT2K(vector<MPZ> A, vector<MPZ> B){
	for(int i = 0; i < A.size()-1; ++i){
		cout << A[i] << ", " << A[i+1] << endl;
		cout << B[i] << ", " << B[i+1] << endl;
		vector<MPZ> TA(2);
		TA[0] = A[i];
		TA[1] = A[i+1];
		vector<MPZ> TB(2);
		TB[0] = B[i];
		TB[1] = B[i+1];
		A[i+1] = CRT(TA, TB);
		B[i+1] = B[i]*B[i+1];
	}
	return A[A.size()-1];
}

MPZ CRT2KCOURS(vector<MPZ> A, vector<MPZ> B){
	MPZ N = 1;
	MPZ res = 0;
	for(int i = 0; i < B.size(); ++i)
		N *= B[i];
	for(int i = 0; i < A.size()-1; ++i){
		MPZ ni = N / B[i];
		MPZ ui = Euclid(N, ni)[0];
		res += A[i] * ui * ni;
	}
	return A[A.size()-1];
}

vector<MPZ> Lagrange(vector<MPZ> X, vector<MPZ> Y){
	vector<MPZ> res(X.size() - 1);
	res[0] = 1;
		
	cout << "début x" << endl;
	for(int l = 0; l < X.size(); ++l)
		cout << X[l] << endl;
	cout << "fin x" << endl;
		
	cout << "début y" << endl;
	for(int l = 0; l < Y.size(); ++l)
		cout << Y[l] << endl;
	cout << "fin y" << endl;
	
	for(int i = 0; i < X.size(); ++i){
		
		vector<MPZ> globalTempPoly(1);
		globalTempPoly[0] = 1;
		MPZ yi = Y[i], temp = 1;
		
		for(int j = 0; j < X.size(); ++j){
			if(i == j)
				continue;
			temp *= (X[i] - X[j]);
		}
		
		cout << "temp = " << temp << endl;
		cout << "yi = " << yi << endl;
				
		for(int j = 0; j < X.size(); ++j){
			if(i == j)
				continue;
			vector<MPZ> tempPoly(2);
			tempPoly[1] = 1;
			tempPoly[0] = X[j]*(-1);
		
			cout << "début tempPoly au tour de boucle " << i << endl;
			for(int l = 0; l < tempPoly.size(); ++l)
				cout << tempPoly[l] << endl;
			cout << "fin tempPoly" << endl;
			
			globalTempPoly = model::mult(globalTempPoly, tempPoly);
		
			cout << "début globalTempPoly au tour de boucle " << i  << endl;
			for(int l = 0; l < globalTempPoly.size(); ++l)
				cout << globalTempPoly[l] << endl;
			cout << "fin globalTempPoly" << endl;
		}
		
		cout << "apès la boucle : début globalTempPoly" << endl;
		for(int l = 0; l < globalTempPoly.size(); ++l)
			cout << globalTempPoly[l] << endl;
		cout << "apès la boucle : fin globalTempPoly" << endl;
		
		cout << "yi / temp =" << (yi/temp) << endl;
		
		model::mult(globalTempPoly, (yi / temp));
		
		
		
		res = model::add(res, globalTempPoly);
		
		
		cout << "après le " << i << "-ème tour de boucle, on a " << endl;
		
		cout << "début res" << endl;
		for(int l = 0; l < res.size(); ++l)
			cout << res[l] << endl;
		cout << "fin res" << endl;
		
	}
}

int main(int argc, char **argv)

{ 
	MPZ a(1), b(2);
	cout << (a+b) << endl;
	
	/*vector<MPZ> A(8);
	for(int i = 0; i < 8; ++i)
		A[i] = pow(256, i) + pow(i, 256);
	
	vector<MPZ> B(8);
	for(int i = 0; i < 8; ++i){
		MPZ a(pow(256, i));
		MPZ b(pow(i, 256));
		B[i] = (a+b)%43;
	}*/
	
	//cout << CRT2KCOURS(A, B) << endl;
	
	vector<MPZ> A(3);
	A[0] = 0;
	A[1] = 1;
	A[2] = -1;
	vector<MPZ> B(3);
	B[0] = 1;
	B[1] = 4;
	B[2] = 0;
	
	vector<MPZ> res = Lagrange(A, B);
	for(int i = 0; i < res.size(); ++i)
		cout << res[i] << endl;

    return 0;

}
