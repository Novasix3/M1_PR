package TP5;

import java.math.BigInteger;

public class ElGamal {
	static BigInteger ZERO = BigInteger.ZERO;
	static BigInteger ONE = BigInteger.ONE;
	
	public static void main(String[] args) {
		Pair<Triplet, BigInteger> keys = keyGenerator();
		BigInteger message = NumberGenerator.uniformRandom(ZERO, keys.r.p.subtract(ONE));
		Pair<BigInteger, BigInteger> signature = signature(message, keys.r, keys.s);
		System.out.println(message);
		System.out.println(verification(message, signature, keys.r));
	
	}
	
	public static Pair<Triplet, BigInteger> keyGenerator() {
		BigInteger p = new BigInteger("100000007");
		BigInteger g = new BigInteger("5");
		BigInteger x = NumberGenerator.uniformRandom(ZERO, p.subtract(ONE));
		BigInteger y = g.modPow(x, p);
		return new Pair<Triplet, BigInteger>(new Triplet(p, g, y), x);
	}
	
	public static Pair<BigInteger, BigInteger> signature(BigInteger m, Triplet publicKey, BigInteger secretKey) {
		System.out.println("in signature");
		System.out.println("m = "+m);
		BigInteger k = NumberGenerator.uniformRandom(ZERO, publicKey.p.subtract(ONE));
		System.out.println("k = "+k);
		BigInteger r = publicKey.g.modPow(k, publicKey.p);
		System.out.println("r = "+r);
		System.out.println("x = "+secretKey);
		BigInteger top = m.subtract(secretKey.multiply(r));
		System.out.println("top = "+top);
		BigInteger s = top.divide(k).mod(publicKey.p.subtract(ONE));
		System.out.println("s = "+s);
		return new Pair<BigInteger, BigInteger>(r, s);
	}
	
	public static boolean verification(BigInteger m, Pair<BigInteger, BigInteger> signature, Triplet publicKey) {
		System.out.println("in verif");
		
		if(signature.r.compareTo(publicKey.p) >= 0)
			return false;
		

		System.out.println("avant seconde vérif");
		
		if(signature.s.compareTo(publicKey.p.subtract(ONE)) >= 0)
			return false;
		

		System.out.println("avant calcul");
		System.out.println("on a : ");
		System.out.println("p = "+publicKey.p);
		System.out.println("g = "+publicKey.g);
		System.out.println("y = "+publicKey.y);
		System.out.println("r = "+signature.r);
		System.out.println("s = "+signature.s);
		System.out.println("m = "+m);
		
		
		BigInteger gm = publicKey.g.modPow(m, publicKey.p);
		
		System.out.println("gm = "+gm);
		
		BigInteger yr = publicKey.y.modPow(signature.r, publicKey.p);
		
		System.out.println("yr = "+yr);
		
		BigInteger rs = signature.r.modPow(signature.s, publicKey.p);
		
		System.out.println("rs = "+rs);
		
		BigInteger right = yr.multiply(rs).mod(publicKey.p);
		
		System.out.println("right = "+right);
		

		System.out.println("avant troisième vérif");
		
		if(gm.compareTo(right) == 0)
			return true;
		
		System.out.println("fail");
		
		return false;
	}
}
