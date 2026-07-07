// Arup Guha- Edited by Carter Gerlach- Fall 2025.
// Originally written in Fall 2006, edited on 10/23/07 for CIS 3362
import java.math.BigInteger;
import java.util.*;

public class MillerRabin {
	
	public static void main(String[] args) {

		Random r = new Random();
		int trials = 1000000; // 1 Million trials, this can be changed
		int[] fermatFreq = new int[51];
		int[] millerFreq = new int[51];

		for (int i = 0; i < trials; i++) {
			BigInteger n = randomComposite(r);

			// The Fermat Test
			int fermatCount = 0;
			for (; fermatCount < 50; fermatCount++) {
				if (!FermatTest(n, r)) break;
			}
			fermatFreq[fermatCount]++;

			// The Miller-Rabin Test
			int millerCount = 0;
			for (; millerCount < 50; millerCount++) {
				if (!MyMillerRabin(n, r)) break;
			}
			millerFreq[millerCount]++;
		}
		
		System.out.println("Number of trials: " + trials);
		
		// Print the results
		System.out.println("Fermat Test Frequency:");
		for (int i = 0; i <= 50; i++) {
			if (fermatFreq[i] > 0)
				System.out.println(i + ": " + fermatFreq[i]);
		}
		
		System.out.println("Miller-Rabin Test Frequency:");
		for (int i = 0; i <= 50; i++) {
			if (millerFreq[i] > 0)
				System.out.println(i + ": " + millerFreq[i]);
		}


	}

	public static BigInteger randomComposite(Random random) {
		while (true) {
			// Generate a random long between 10^8 and 10^9
			long num = (long)(random.nextDouble() * (1000000000L - 100000000L)) + 100000000L;

			// Check if candidate is not divisible by 2, 3, or 5 and is composite
			// If number does not meet criteria, it re-generates
			if (num % 2 != 0 && num % 3 != 0 && num % 5 != 0 &&
				!BigInteger.valueOf(num).isProbablePrime(20)) {
				return BigInteger.valueOf(num);
			}
		}
	}

	
	public static boolean FermatTest(BigInteger n, Random r) {
		
		// Ensures that temp > 1 and temp < n.
		BigInteger temp = BigInteger.ZERO;
		do {
			temp = new BigInteger(n.bitLength()-1, r);
		} while ((temp.compareTo(BigInteger.ONE) <= 0) || !n.gcd(temp).equals(BigInteger.ONE));  // added condition to ensure skip values where gcd(a, n) != 1.
		
		// Just calculate temp^*(n-1) mod n
		BigInteger ans = temp.modPow(n.subtract(BigInteger.ONE), n);
		
		// Return true iff it passes the Fermat Test!
		return (ans.equals(BigInteger.ONE));
	}
	
	private static boolean MyMillerRabin(BigInteger n, Random r) {
		
		// Ensures that temp > 1 and temp < n.
		BigInteger temp = BigInteger.ZERO;
		do {
			temp = new BigInteger(n.bitLength()-1, r);
		} while ((temp.compareTo(BigInteger.ONE) <= 0) || !n.gcd(temp).equals(BigInteger.ONE)); // // added condition to ensure skip values where gcd(a, n) != 1.
		
		// Screen out n if our random number happens to share a factor with n.
		if (!n.gcd(temp).equals(BigInteger.ONE)) return false;
		
		// For debugging, prints out the integer to test with.
		//System.out.println("Testing with " + temp);
		
		BigInteger base = n.subtract(BigInteger.ONE);
		BigInteger TWO = new BigInteger("2");
		
		// Figure out the largest power of two that divides evenly into n-1.
		int k=0;
		while ( (base.mod(TWO)).equals(BigInteger.ZERO)) {
			base = base.divide(TWO);
			k++;
		}
		
		// This is the odd value r, as described in our text.
		//System.out.println("base is " + base);
		
		BigInteger curValue = temp.modPow(base,n);
		
		// If this works out, we just say it's prime.
		if (curValue.equals(BigInteger.ONE))
			return true;
			
		// Otherwise, we will check to see if this value successively 
		// squared ever yields -1.
		for (int i=0; i<k; i++) {
			
			// We need to really check n-1 which is equivalent to -1.
			if (curValue.equals(n.subtract(BigInteger.ONE)))
				return true;
				
			// Square this previous number - here I am just doubling the 
			// exponent. A more efficient implementation would store the
			// value of the exponentiation and square it mod n.
			else
				curValue = curValue.modPow(TWO, n);
		}
		
		// If none of our tests pass, we return false. The number is 
		// definitively composite if we ever get here.
		return false;
	}
	
}
