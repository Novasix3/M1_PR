#include <iostream>

#include <gmpxx.h> //On inclut la bibliothèque gmp
#include <chrono>
#include <vector>
#include "Arithmpz.h"

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
		cout << "R[0] = " << R[0] << endl;
		cout << "R[1] = " << R[1] << endl;
		cout << "U[0] = " << U[0] << endl;
		cout << "U[1] = " << U[1] << endl;
		cout << "V[0] = " << V[0] << endl;
		cout << "V[1] = " << V[1] << endl;
		cout << "\n" << endl;
		cout << R[0] / R[1] << endl ;
		MPZ q(R[0]/R[1]);
		vector<MPZ> temp(3);
		temp[0] = R[0] - q*R[1];
		temp[1] = U[0] - q*U[1];
		temp[2] = V[0] - q*V[1];
		cout << "temp[0] = " << temp[0] << endl;
		cout << "temp[1] = " << temp[1] << endl;
		cout << "temp[2] = " << temp[2] << endl;
		cout << "\n\n\n" << endl;
	}
	vector<MPZ> res(2);
	res[0] = U[0];
	res[1] = U[1];
	return res;
}

MPZ CRT(vector<MPZ> A, vector<MPZ> B){
	vector<MPZ> Bezout = Euclid(B[0], B[1]);
	cout << Bezout[0] << ", " << Bezout[1] << endl;
	
}

int main(int argc, char **argv)

{ 
	
	vector<MPZ> A(2);
	A[0] = 0;
	A[1] = 1;
	
	vector<MPZ> B(2);
	B[0] = 4;
	B[1] = 5;
	
	cout << CRT(A, B) << endl;

    return 0;

}
