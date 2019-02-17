package TP1;

import java.util.ArrayList;
import java.util.HashMap;

public class Vigenere_decrypt {
	public static void main(String[] args) {
		String s = "zbpuevpuqsdlzgllksousvpasfpddggaqwptdgptzweemqzrdjtddefek" + 
				"eferdprrcyndgluaowcnbptzzzrbvpssfpashpncotemhaeqrferdlrlw" + 
				"wertlussfikgoeuswotfdgqsyasrlnrzppdhtticfrciwurhcezrpmhtp" + 
				"uwiyenamrdbzyzwelzucamrptzqseqcfgdrfrhrpatsepzgfnaffisbpv" + 
				"dblisrplzgnemswaqoxpdseehbeeksdptdttqsdddgxurwnidbdddplncsd";
		
		int keyLength = kasiski(s);
		decrypt(s, keyLength);
	}
	
	public static void decrypt(String s, int keyLength) {
		if(keyLength == 0) {
			for(int i = 0; i < s.length(); ++i) {
				
			}
		}
	}
	
	public static int kasiski(String s) {
		HashMap<String, ArrayList<Integer>> map = new HashMap<>();
		for(int i = 0; i < s.length()-1; ++i) {
			String s1 = String.valueOf(s.charAt(i))+String.valueOf(s.charAt(i+1));
			if(map.containsKey(s1))
				continue;
			ArrayList<Integer> temp = new ArrayList<>();
			temp.add(i);
			map.put(s1, temp);
			for(int j = i+1; j < s.length()-1; ++j) {
				String s2 = String.valueOf(s.charAt(j))+String.valueOf(s.charAt(j+1));
				if(s1.equals(s2)) {
					temp.add(j);
					map.replace(s1, temp);
				}
			}
			System.out.println(temp);
		}
		HashMap<String, Integer> m = new HashMap<String, Integer>();
		for(String e : map.keySet()) 
			m.put(e, pgcd(map.get(e)));
		
		System.out.println(m);

		HashMap<Integer, Integer> m2 = new HashMap<Integer, Integer>();
		for(Integer e : m.values()) {
			if(m2.get(e) != null)
				m2.replace(e, m2.get(e)+1);
			else
				m2.put(e, 0);
		}
		
		System.out.println(m2);
		
		int keyMax = 0, valMax = 0;
		for(Integer e : m2.keySet()) {
			int t = m2.get(e);
			if(e != 0 && t > keyMax) {
				keyMax = t;
				valMax = e;
			}
		}
		System.out.println(valMax);
		return valMax;
	}
	
	public static int pgcd(ArrayList<Integer> l) {
		if(l.size() == 1)
			return 0;
		//System.out.println(l);
		int pgcd = l.get(1) - l.get(0);
		for(int i = 1; i < l.size()-1; ++i) 
			pgcd = pgcd(pgcd, l.get(i+1) - l.get(i));
		if(pgcd > 26)
			return 0;
		return pgcd;
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
