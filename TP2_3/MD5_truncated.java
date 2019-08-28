package TP2_3;

import java.util.Hashtable;

public class MD5_truncated {
	static Hashtable<String, String> map = new Hashtable<>();
	static int iter = 0;

	public static void main(String[] args) {
		StringBuilder s = new StringBuilder("Collision MD5 ????????????");

		// fill2(s, 14);
		// toHexString(computeMD5(s.getBytes()))
		fill(preprocess(s), s.indexOf("?"));
	}

	public static StringBuilder preprocess(StringBuilder s) {
		// System.out.println("s = " + s);
		for (int i = s.indexOf("?"); i < s.length(); ++i)
			s.replace(i, i + 1, "a");
		// System.out.println("s = " + s);
		return s;
	}

	public static void fill(StringBuilder s, int pos) {
		for (int i = 0; i < 26; ++i) {
			if (pos < s.length() - 1) {
				fill(s, pos + 1);
				s.replace(pos, pos + 1, Character.toString('a' + i));
			} else {
				s.replace(pos, pos + 1, Character.toString('a' + i));
				addHash(s);
			}
		}
	}
	
	public static void addHash(StringBuilder s) {
		//System.out.println("on va générer le hash de : " + s);
		StringBuilder temp = new StringBuilder(toHexString(computeMD5((s.toString()).getBytes())));
		//System.out.println("le hash généré est : \"" + temp + "\".");
		temp = temp.delete(6, temp.length());
		//System.out.println("le hash coupé est : \"" + temp + "\".");
		String t = map.get(temp.toString());
		//System.out.println("ce qu'on a récupéré de la table de hash pour la clé \"" + temp + "\" est : " + t);

		if (t == null) {
			//System.out.println("on ajoute à la table de hash le couple :  (" + temp + ", " + s + ").");
			map.put(temp.toString(), s.toString());
			iter++;
		} else {
			if(!t.equals(s.toString())) {
				System.out.println("Les deux chaînes suivantes ont générés le même hash :");
				System.out.println(t);
				System.out.println(s);
				System.exit(0);
			}
		}
		/*if(iter >= 45656) {
			System.out.println("on a fait "+iter+", on check si y'a pas d'erreur.");
			System.out.println("la taille de la map est :"+map.size());
			//System.out.println(map);
			System.exit(0);
		}*/
	}

	public static void fill2(StringBuilder s, int pos) {
		System.out.println("test de s = " + s);
		// System.out.println("pos = "+pos+", s.length = "+s.length());
		for (int i = 0; i < 26; ++i) {
			if (pos < s.length()) {
				System.out.println("on passe de la string " + s);
				s.replace(pos, pos + 1, Character.toString('a' + i));
				System.out.println("à  " + s);
				fill2(s, pos + 1);
			} else {
				if (pos < s.length())
					s.replace(pos, pos + 1, Character.toString('a' + i));
				System.out.println("on va générer le hash de : " + s);
				StringBuilder temp = new StringBuilder(toHexString(computeMD5((s.toString()).getBytes())));
				System.out.println("le hash généré est : " + temp);
				temp.delete(6, temp.length());
				System.out.println("le hash coupé est : " + temp);
				String t = map.get(temp.toString());

				if (t == null) {
					System.out.println("on ajoute à la table de hash : " + temp);
					map.put(temp.toString(), s.toString());
				} else {
					if (t.equals(s.toString())) {
						System.out.println("Les deux chaînes suivantes ont générés le même hash :");
						System.out.println(temp);
						System.out.println(s);
						System.exit(0);
					}
				}
			}
		}
	}

	public static String toHexString(byte[] b) {
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < b.length; i++) {
			sb.append(String.format("%02X", b[i] & 0xFF));
		}
		return sb.toString();
	}

	public static byte[] hash(byte[] message) {
		int messageLenBytes = message.length;
		int numBlocks = ((messageLenBytes + 8) >>> 6) + 1;
		int totalLen = numBlocks << 6;
		byte[] paddingBytes = new byte[totalLen - messageLenBytes];
		paddingBytes[0] = (byte) 0x80;

		long messageLenBits = (long) messageLenBytes << 3;
		for (int i = 0; i < 8; i++) {
			paddingBytes[paddingBytes.length - 8 + i] = (byte) messageLenBits;
			messageLenBits >>>= 8;
		}
		int[] buffer = new int[16];
		int[] r = new int[64];
		for (int i = 0; i < 16; ++i) {
			if (i % 4 == 0) {
				r[i] = 7;
				r[i + 16] = 5;
				r[i + 32] = 4;
				r[i + 48] = 6;
			} else if (i % 4 == 1) {
				r[i] = 12;
				r[i + 16] = 9;
				r[i + 32] = 11;
				r[i + 48] = 10;
			} else if (i % 4 == 2) {
				r[i] = 17;
				r[i + 16] = 14;
				r[i + 32] = 16;
				r[i + 48] = 15;
			} else {
				r[i] = 22;
				r[i + 16] = 20;
				r[i + 32] = 23;
				r[i + 48] = 21;
			}
		}
		int[] k = new int[64];
		for (int i = 0; i < k.length; ++i)
			k[i] = (int) Math.floor(Math.sin(i + 1) * Math.pow(2, 32));
		int h0 = 0x67452301;
		int h1 = (int) 0xEFCDAB89L;
		int h2 = (int) 0x98BADCFEL;
		int h3 = 0x10325476;

		for (int j = 0; j < 64; ++j) {
			int a = h0;
			int b = h1;
			int c = h2;
			int d = h3;

			int f, g;

			for (int i = 0; i < 64; ++i) {
				int div16 = j >>> 4;
				if (i <= 15) {
					f = (b & c) | (~b & d);
					g = i;
				} else if (i <= 31) {
					f = (b & d) | (c & ~d);
					g = (5 * i + 1) % 16;
				} else if (i <= 47) {
					f = b ^ c ^ d;
					g = (3 * i + 5) % 16;
				} else {
					f = c ^ (b | ~d);
					g = (7 * i) % 16;
				}
				a = d;
				d = c;
				c = b;
				int temp = b + Integer.rotateLeft(a + f + buffer[g] + k[j], r[(div16 << 2) | (j & 3)]);
				b = temp;
			}
			h0 += a;
			h1 += b;
			h2 += c;
			h3 += d;
		}
		byte[] md5 = new byte[16];
		int count = 0;
		for (int i = 0; i < 4; i++) {
			int n = (i == 0) ? h0 : ((i == 1) ? h1 : ((i == 2) ? h2 : h3));
			for (int j = 0; j < 4; j++) {
				md5[count++] = (byte) n;
				n >>>= 8;
			}
		}
		return md5;
	}

	private static final int INIT_A = 0x67452301;
	private static final int INIT_B = (int) 0xEFCDAB89L;
	private static final int INIT_C = (int) 0x98BADCFEL;
	private static final int INIT_D = 0x10325476;

	private static final int[] SHIFT_AMTS = { 7, 12, 17, 22, 5, 9, 14, 20, 4, 11, 16, 23, 6, 10, 15, 21 };

	private static final int[] TABLE_T = new int[64];
	static {
		for (int i = 0; i < 64; i++)
			TABLE_T[i] = (int) (long) ((1L << 32) * Math.abs(Math.sin(i + 1)));
	}

	public static byte[] computeMD5(byte[] message) {
		int messageLenBytes = message.length;
		int numBlocks = ((messageLenBytes + 8) >>> 6) + 1;
		int totalLen = numBlocks << 6;
		byte[] paddingBytes = new byte[totalLen - messageLenBytes];
		paddingBytes[0] = (byte) 0x80;

		long messageLenBits = (long) messageLenBytes << 3;
		for (int i = 0; i < 8; i++) {
			paddingBytes[paddingBytes.length - 8 + i] = (byte) messageLenBits;
			messageLenBits >>>= 8;
		}

		int a = INIT_A;
		int b = INIT_B;
		int c = INIT_C;
		int d = INIT_D;
		int[] buffer = new int[16];
		for (int i = 0; i < numBlocks; i++) {
			int index = i << 6;
			for (int j = 0; j < 64; j++, index++)
				buffer[j >>> 2] = ((int) ((index < messageLenBytes) ? message[index]
						: paddingBytes[index - messageLenBytes]) << 24) | (buffer[j >>> 2] >>> 8);
			int originalA = a;
			int originalB = b;
			int originalC = c;
			int originalD = d;
			for (int j = 0; j < 64; j++) {
				int div16 = j >>> 4;
				int f = 0;
				int bufferIndex = j;
				switch (div16) {
				case 0:
					f = (b & c) | (~b & d);
					break;

				case 1:
					f = (b & d) | (c & ~d);
					bufferIndex = (bufferIndex * 5 + 1) & 0x0F;
					break;

				case 2:
					f = b ^ c ^ d;
					bufferIndex = (bufferIndex * 3 + 5) & 0x0F;
					break;

				case 3:
					f = c ^ (b | ~d);
					bufferIndex = (bufferIndex * 7) & 0x0F;
					break;
				}
				int temp = b + Integer.rotateLeft(a + f + buffer[bufferIndex] + TABLE_T[j],
						SHIFT_AMTS[(div16 << 2) | (j & 3)]);
				a = d;
				d = c;
				c = b;
				b = temp;
			}

			a += originalA;
			b += originalB;
			c += originalC;
			d += originalD;
		}

		byte[] md5 = new byte[16];
		int count = 0;
		for (int i = 0; i < 4; i++) {
			int n = (i == 0) ? a : ((i == 1) ? b : ((i == 2) ? c : d));
			for (int j = 0; j < 4; j++) {
				md5[count++] = (byte) n;
				n >>>= 8;
			}
		}
		return md5;
	}
}
