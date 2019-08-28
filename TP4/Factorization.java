package TP4;

import java.math.BigInteger;

public class Factorization {
	static BigInteger ZERO = BigInteger.ZERO;
	static BigInteger ONE = BigInteger.ONE;
	static BigInteger TWO = (ONE).add(ONE);
	static BigInteger THREE = (ONE).add(TWO);
	
	public static void main(String[] args) {
		BigInteger n = new BigInteger("3148240326296492491829836036538028522262397298543021512290073");
		
		factoDiv(n, THREE);
		

		n = new BigInteger("4433634977317959977189716351978918572296527677331175210881861");
		
		factoFermat(n);
	}
	
	
	public static boolean factoDiv(BigInteger n, BigInteger start) {
		if(RSANumberGenerator.first_test(n)) {
			System.out.println(n+" est premier");
			return true;
		}
		if(n.mod(TWO).equals(ZERO)) {
			System.out.print(TWO+" * "+n.divide(TWO));
			return true;
		}
		if(n.equals(BigInteger.ONE))
			return true;
		for (BigInteger i = start; !i.equals(new BigInteger("10000000001")); i = i.add(TWO)) {
			if(n.mod(i).equals(BigInteger.ZERO)) {
				System.out.println(i+" divise "+n);
				System.out.println("et "+n.divide(i)+" est premier, donc ce sont les deux nombres de la facto de n.");
				return true;
				/*if(factoDiv(n.divide(i), i)) {
					System.out.print(i+" * ");
					return true;
				}*/
			}/*else if(i.mod(new BigInteger("1000001")).equals(ZERO)){
				System.out.println(i+" ne divise pas "+n);
			}*/
		}
		System.out.println("Le nombre n'a pas de factorisation.");
		return false;
	}
	
	public static void factoFermat(BigInteger n) {
		if(n.mod(TWO).equals(ZERO)) {
			System.out.print(TWO+" * "+n.divide(TWO));
			return ;
		}
		BigInteger a = n.sqrt();
		System.out.println("a = "+a);
		if(a.multiply(a).compareTo(n) == 0)
			System.out.println(a+" * "+a);
		a = a.add(ONE);
		System.out.println("a = "+a);
		BigInteger b = a.multiply(a).subtract(n).sqrt();
		while(b.multiply(b).compareTo(a.multiply(a).subtract(n)) != 0) {
			System.out.println("a = "+a);
			a = a.add(ONE);
			b = a.multiply(a).subtract(n).sqrt();
			System.out.println("b = "+b);
		}
		System.out.println((a.subtract(b))+" * "+(a.add(b))+" donne "+n);
	}
}
