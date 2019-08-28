package TP5;

/*
 * See "ClassroomInterval" problem
 */

public class Pair<K, V> {
	K r;
	V s;
	
	Pair(K r, V s){
		this.r = r;
		this.s = s;
	}
	
	public String toString() {
		return "("+r+","+s+")";
	}
}
