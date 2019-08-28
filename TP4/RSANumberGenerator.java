package TP4;
import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Random;

public class RSANumberGenerator {
	static int ACCURACY = 7; // Integer for the Miller-Rabin primality test accuracy

	// BigInteger constant, for better readability
	static BigInteger ZERO = BigInteger.ZERO;
	static BigInteger ONE = BigInteger.ONE;
	static BigInteger TWO = (ONE).add(ONE);
	static BigInteger THREE = (ONE).add(TWO);

	public static void main(String[] args) {
		/*
		 * test_miller_rabin(57); test_miller_rabin(848885151);
		 * test_miller_rabin(8484887); test_miller_rabin(561); test_miller_rabin(577);
		 * test_miller_rabin("393050634124102232869567034555427371542904833"); 
		 * //A Cullen prime 
		 * test_miller_rabin("8683317618811886495518194401279999999");
		 * //A factorial prime
		 */
		// test_TMR();
		// errorRateMRTest();
		gen_rsa(30);
		gen_rsa(70);
		gen_rsa(170);
	}
	
	public static BigInteger sqrt(BigInteger x) {
		BigInteger div = BigInteger.ZERO.setBit(x.bitLength() / 2);
		BigInteger div2 = div;
		// Loop until we hit the same value twice in a row, or wind
		// up alternating.
		while (true) {
			BigInteger y = div.add(x.divide(div)).shiftRight(1);
			if (y.equals(div) || y.equals(div2))
				return y;
			div2 = div;
			div = y;
		}
	}
	
	public static boolean first_test(BigInteger N) {
		// We get the square root of N, rounded up
		BigInteger sqrtN = sqrt(N).add(ONE);
		
		//return N.isProbablePrime(20);

		for (BigInteger i = TWO; !i.equals(sqrtN); i = i.add(ONE)) {
			if (N.mod(i).equals(ZERO))
				return false;
		}

		// If not a single number between 2 and N^(0.5) divide N, N is prime
		return true;
	}
	
	public static ArrayList<BigInteger> getPrimeList() {
		ArrayList<BigInteger> l = new ArrayList<>();
		for (BigInteger i = new BigInteger("1"); !i.equals(new BigInteger("10000000")); i = i.add(ONE)) {
			System.out.println("i = "+i);
			if (first_test(i))
				l.add(i);
		}
		return l;
	}

	// RNG between bottom and top
	public static BigInteger uniformRandom(BigInteger bottom, BigInteger top) {
		if (bottom.equals(top))
			return bottom;
		Random rnd = new Random();
		BigInteger res;
		do {
			res = new BigInteger(top.bitLength(), rnd);
		} while (res.compareTo(bottom) < 0 || res.compareTo(top) > 0);
		return res;
	}

	// Question A :
	// true = probably primer, false = composed
	public static boolean test_miller_rabin(BigInteger N) {
		if (N.equals(TWO) || N.equals(THREE)) { // If N is 2 or 3
			// System.out.println("N = " + N + " is prime.");
			return true;
		} else if (N.compareTo(ONE) <= 0) { // If N is 0 or negative
			// System.out.println("N = " + N + " is an invalid entry.");
			return false;
		} else if (N.mod(TWO).equals(ZERO)) {
			// System.out.println("N = " + N + " is composed."); // If N is even
			return false;
		}

		// Constant : NM1 = N-1, m = N-1, h = 0
		BigInteger NM1 = N.subtract(ONE), m = NM1, h = ZERO, b;

		// m and h are used to count how many times 2 divide N-1
		while ((m.mod(TWO)).equals(ZERO)) {
			h = h.add(ONE);
			m = m.divide(TWO);
		}
		// Now, we know that m is odd and n = 1 + (2^h)*m

		// We do ACCURACY tests,
		for (int i = 1; i <= ACCURACY; ++i) {
			BigInteger a = uniformRandom(TWO, NM1); // a is now an integer between 2 and N-1
			b = a.modPow(m, N); // b = a^m [N]
			if (!b.equals(ONE) && !b.equals(NM1)) { // If b is not 1 or N-1
				for (BigInteger j = ONE; j.compareTo(h.subtract(ONE)) <= 0 && !b.equals(NM1); j = j.add(ONE)) {
					b = b.modPow(TWO, N); // b = b^2 [N]
					if (b.equals(ONE)) // if b == 1
						return false;
				}
				if (!b.equals(NM1)) { // If b is not N-1, we are sure N is composed
					// System.out.println("N = " + N + " is composed.");
					return false;
				}
			}
		}
		// If we get to that point, we know that N is probably prime
		// System.out.println("N = " + N + " is probably prime.");
		return true;
	}

	// Simple redirection function
	public static boolean test_miller_rabin(int N) {
		return test_miller_rabin(BigInteger.valueOf(N));
	}

	public static boolean test_miller_rabin(long N) {
		return test_miller_rabin(BigInteger.valueOf(N));
	}

	public static boolean test_miller_rabin(String N) {
		return test_miller_rabin(new BigInteger(N));
	}
	
	public static BigInteger my_gcd(BigInteger a, BigInteger b) {
		// System.out.println("a = "+a+", b = "+b);
		if (b.equals(ZERO))
			return a;
		return my_gcd(b, a.mod(b));
	}
	
	public static BigInteger my_inverse(BigInteger a, BigInteger N) {
		// Setup of the variables
		BigInteger r0 = a, r1 = N;
		BigInteger u0 = ONE, u1 = ZERO;
		BigInteger v0 = ZERO, v1 = ONE;

		// Main loop of the algorithm
		while (!r1.equals(ZERO)) {
			BigInteger q = r0.divide(r1);
			BigInteger rs = r0, us = u0, vs = v0;
			r0 = r1;
			u0 = u1;
			v0 = v1;
			r1 = rs.subtract(q.multiply(r1));
			u1 = us.subtract(q.multiply(u1));
			v1 = vs.subtract(q.multiply(v1));
		}

		// a is not invertible in Z/nZ
		if (r0.compareTo(ONE) > 0)
			return ZERO;

		// Return (u0+N)%N
		return (u0.add(N)).mod(N);
	}

	// Question D :
	public static Triplet gen_rsa(int t) {
		// if t is invalid, return null
		if (t < 1)
			return null;

		System.out.println("We're gonna generate two primes numbers between 2^" + (t - 1) + " and 2^" + t + ".");

		BigInteger twoMinusOneT = TWO.pow(t - 1); // 2^(t-1)
		BigInteger twoT = TWO.pow(t); // 2^t

		// We pick p between 2^(t-1) and 2^t
		BigInteger p;
		do
			p = uniformRandom(twoMinusOneT, twoT);
		while (!test_miller_rabin(p));
		// p is now probably prime

		System.out.println("The first prime integer generated is p = " + p + ".");

		// We pick q between 2^(t-1) and 2^t
		BigInteger q;
		do
			q = uniformRandom(twoMinusOneT, twoT);
		while (!test_miller_rabin(q));
		// q is now probably prime

		System.out.println("The second prime integer generated is q = " + q + ".");

		BigInteger res = p.multiply(q);
		System.out.println("So the result of p*q is " + res + ".");
		
		// return p*q
		return new Triplet(res, p, q);
	}
}
