package TP1;

public class affine_decrypt {
	public static void main(String[] args) {
		String s = "ntjmpumgxpqtstgqpgtxpnchumtputgfsftgthnngxnchumwx"
				+ "ootrtumhpyctgktjqtjchfooxujqhgztumxpotjxotfoqtohr"
				+ "xumhzutwftgtopfmntjmpuatmfmshodpfrxpjjtqtghbxuj";

		decrypt(s);
	}

	public static void decrypt(String s) {
		for(int i = 0; i < 27; ++i) {
			if(pgcd(i, 26) == 1) {
				for(int k = 0; k < 27; ++k) {
					if(i == 9 && k == 7)
						System.out.println("\nici");
					for(int j = 0; j < s.length(); ++j) {
							System.out.print(Character.toString(print(s.charAt(j), i, k)));
					}
					if(i == 9 && k == 7)
						System.out.println("\nici");
					System.out.println();
				}
			}
		}
	}
	
	public static int print(int c, int i, int k) {
		int diff = c - 'a' - k;
		diff = diff * i;
		while(diff < 0)
			diff += 26;
		diff = diff % 26;
		return diff + 'a';
	}

	public static int pgcd(int m, int n) {
		int r = 0;
		while (n != 0) {
			r = m % n;
			m = n;
			n = r;
		}
		return m;
	}
}
