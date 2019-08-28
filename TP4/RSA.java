package TP4;

import java.math.BigInteger;

public class RSA {
	
	static BigInteger ZERO = BigInteger.ZERO;
	static BigInteger ONE = BigInteger.ONE;
	static BigInteger TWO = (ONE).add(ONE);
	static BigInteger THREE = (ONE).add(TWO);
	
	public static void main(String[] args) {
		Quintuplet keys = generateKeys(32);
		System.out.println("N = "+keys.N);
		System.out.println("p = "+keys.p);
		System.out.println("q = "+keys.q);
		System.out.println("e = "+keys.e);
		System.out.println("d = "+keys.d);
		
		BigInteger m = new BigInteger("485692");
		
		BigInteger c = encrypt(m, keys.e, keys.N);
		System.out.println("c = "+c);
		
		BigInteger res = decrypt(c, keys.d, keys.N);
		System.out.println("res = "+res);
	}
	
	public static Quintuplet generateKeys(int k) {
		Triplet t = RSANumberGenerator.gen_rsa(k);
		
		BigInteger p1 = (t.p).subtract(BigInteger.ONE);
		BigInteger q1 = (t.q).subtract(BigInteger.ONE);
		BigInteger phiN = p1.multiply(q1);
		
		BigInteger e;
		do
			e = RSANumberGenerator.uniformRandom(TWO, phiN);
		while (RSANumberGenerator.my_gcd(phiN, e).compareTo(ONE) != 0);
		
		BigInteger d = e.modInverse(phiN);
		
		return new Quintuplet(t.N, t.p, t.q, e, d);
	}
	
	public static BigInteger encrypt(BigInteger m, BigInteger e, BigInteger N) {
		return m.modPow(e, N);
	}
	
	public static BigInteger decrypt(BigInteger c, BigInteger d, BigInteger N) {
		return c.modPow(d, N);
	}
}
