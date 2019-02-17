package TP1;

public class caesar_decrypt {
	public static void main(String[] args) {
		String s = "vcfgrwqwoizcuwgtowbsobhgoizcuwgsghqseisqsghoixcifr"
				+ "viwxcifrstshsqcaasbhsghqseisjcigbsgojsndogeishobhrsgof"
				+ "hwgobgjcigbsrsjsndogjcigacbhfsfibxcifcijfwsfgobgojcwf"
				+ "zsgwbgwubsgrsjcgdfctsggwcbgdofzshcweiszsghhcbashwsf";
		
		decrypt(s);
	}
	
	public static void decrypt(String s) {
		for(int i = 1; i < 27; ++i) {
			for(int j = 0; j < s.length(); ++j) {
				System.out.print(Character.toString('a' + (s.charAt(j)+i)%26));
			}
			System.out.println();
		}
	}
}
