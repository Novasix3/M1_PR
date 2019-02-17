package TP2_3;

import java.util.Arrays;

public class AES_boxes {
	public static void main(String[] args) {

		/*
		 * Polynomial a = new Polynomial(23);
		 * System.out.println(Integer.toHexString(compute(0x03, 0x04)));
		 * System.out.println(Integer.toHexString(fill(0x03, 0x04)));
		 * System.out.println(Integer.toHexString(a.get())); System.out.println("a = " +
		 * Arrays.toString(getPol(a.get()))); a = Polynomial.inverse(a);
		 * System.out.println("inv de a = " + Arrays.toString(getPol(a.get()))); int b =
		 * affine(a); System.out.println(Integer.toHexString(b));
		 * 
		 * return;
		 */
		int[][] sbox = new int[16][16];
		for (int i = 0; i < 16; ++i) {
			for (int j = 0; j < 16; ++j) {
				sbox[i][j] = forward(i, j);
			}
		}
		for (int i = 0; i < 16; ++i) {
			for (int j = 0; j < 16; ++j)
				System.out.print(Integer.toHexString(sbox[i][j]) + "|");
			System.out.println();
		}
		System.out.println();

		int[][] reverseSbox = new int[16][16];
		for (int i = 0; i < 16; ++i)
			for (int j = 0; j < 16; ++j) {
				int temp = invert(sbox[i][j]);
				System.out.println("temp = " + Integer.toHexString(temp));
				int invi=(temp&(15<<4))>>4;
				int invj=temp&(15);
				reverseSbox[invi][invj] = (i << 4) | j;
			}

		for (int i = 0; i < 16; ++i) {
			for (int j = 0; j < 16; ++j)
				System.out.print(Integer.toHexString(reverseSbox[i][j]) + "|");
			System.out.println();
		}

	}

	public static int forward(int left, int right) {
		int entry = (left << 4) | right;
		int inverse = (left == 0 && right == 0) ? 0 : Polynomial.inverse(new Polynomial(entry)).get();

		int[] pol = new int[8];
		for (int i = 0; i < 8; ++i)
			if ((inverse & (1 << i)) > 0)
				pol[i] = 1;

		int[][] matrix = { { 1, 0, 0, 0, 1, 1, 1, 1 },
							{ 1, 1, 0, 0, 0, 1, 1, 1 },
							{ 1, 1, 1, 0, 0, 0, 1, 1 },
							{ 1, 1, 1, 1, 0, 0, 0, 1 }, 
							{ 1, 1, 1, 1, 1, 0, 0, 0 }, 
							{ 0, 1, 1, 1, 1, 1, 0, 0 },
							{ 0, 0, 1, 1, 1, 1, 1, 0 }, 
							{ 0, 0, 0, 1, 1, 1, 1, 1 } };

		int[] temp = new int[8];
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j < 8; ++j)
				temp[i] += matrix[i][j] * pol[j];

			temp[i] = temp[i] % 2;
		}

		temp[0] = (temp[0] + 1) % 2;
		temp[1] = (temp[1] + 1) % 2;
		temp[5] = (temp[5] + 1) % 2;
		temp[6] = (temp[6] + 1) % 2;

		int res = 0;

		for (int i = 0; i < 8; i++) {
			res = res | (temp[i] << i);
		}

		return res;
	}

	public static int invert(int val) {
		int[] pol = new int[8];
		for (int i = 0; i < 8; ++i)
			if ((val & (1 << i)) > 0)
				pol[i] = 1;
		

		System.out.println("pol = " + Arrays.toString(pol));

		int[][] matrix = {  { 0, 0, 1, 0, 0, 1, 0, 1 }, 
							{ 1, 0, 0, 1, 0, 0, 1, 0 }, 
							{ 0, 1, 0, 0, 1, 0, 0, 1 },
							{ 1, 0, 1, 0, 0, 1, 0, 0 }, 
							{ 0, 1, 0, 1, 0, 0, 1, 0 }, 
							{ 0, 0, 1, 0, 1, 0, 0, 1 },
							{ 1, 0, 0, 1, 0, 1, 0, 0 }, 
							{ 0, 1, 0, 0, 1, 0, 1, 0 } };

		int[] temp = new int[8];
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j < 8; ++j)
				temp[i] += matrix[i][j] * pol[j];
			temp[i] %= 2;
		}

		System.out.println("temp = " + Arrays.toString(temp));

		temp[0] = (temp[0] + 1) % 2;
		temp[2] = (temp[2] + 1) % 2;

		System.out.println("temp after = " + Arrays.toString(temp));

		int res = 0;
		for (int i = 0; i < 8; i++) 
			res = res | (temp[i] << i);
		

		System.out.println("res = " + res);
		//return res;
		return (res == 0) ? 0 : Polynomial.inverse(new Polynomial(res)).get();
	}
}
