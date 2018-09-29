#include <cstddef>
#include <ctime>
#include <cmath>
#include "FHE.h"
#include "EncryptedArray.h"
#include <NTL/ZZX.h>
#include <NTL/ZZ.h>
#include <gmp.h>
#include <iterator>
using namespace std;

class Timer{
	private: clock_t start;
	private: clock_t stop;
	private: double last;
	
	public: Timer(){
		this->start = clock();
	}
	public: double elapsed_time(){
		(*this).stop = clock();
		this->last = ( (double)stop - (double)start )/CLOCKS_PER_SEC;
		return last;
	}
	public: double tillNow(){
		return ( (double)clock() - (double)start )/CLOCKS_PER_SEC;
	}
	public: void reset(){
		start = clock();
	}
};


int main(int argc, char **argv)
{

    /*** BEGIN INITIALIZATION ***/
    long m = 0;                   // Specific modulus
    long p = 2;           // Plaintext base [default=2], should be a prime number
    long r = 10;                   // Lifting [default=1]

    const long LEVEL = 8;         //  if LEVEL==3 then: c1*c2*c3 
    //the final result should be less than p^r, otherwise result := result%(p^r)  Pay Attention To It!
    //the selection of L is probably related to the choice of p and r !
    //L += 2[ 3rlog_2^p/FHE_p2Size ] + 1        Ring-Learning With Errors & HElib of Wenjie Lu
    long L = 17;                  // Number of levels in the modulus chain [default=heuristic]
    long c = 3;                   // Number of columns in key-switching matrix [default=2]
    long hw= 64;                  // Hamming weight of secret key
    long d = 1;                   // Degree of the field extension [default=1]
    long k = 80;                  // Security parameter [default=80] 
    long s = 0;                   // Minimum number of slots [default=0]


    // use the special cyclotomic polynomial where m is a power of 2. PHIm(x) = x^(m/2) +1
    m = FindM(k, L, c, p, d, s, 0);           // m = 8; // PHI8(x) = x^4 + 1     
    cout << "m = " << m << endl;
    FHEcontext context(m, p, r); 	      // Initialize context
    buildModChain(context, L, c);             // Modify the context, adding primes to the modulus chain
    FHESecKey sk(context);                    // Construct a secret key structure
    const FHEPubKey& pk = sk;                 // An "upcast": FHESecKey is a subclass of FHEPubKey
    sk.GenSecKey(hw);                          // Actually generate a secret key with Hamming weight
    addSome1DMatrices(sk);                    // Extra information for relinearization

    // Arrays whose elements will be the coefficients of the polynomials U(x) and V(x)
    vector<long>  u = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    vector<Ctxt> encU(u.size(), Ctxt(pk));

    // Each element is encrypted individually
    for(int i=0; i<u.size(); ++i)
    {
	    Ctxt temp(pk);
	    pk.Encrypt(temp, to_ZZX(u[i]) );
	    encU[i] = temp;
    }

    // The result is stored in res
    Ctxt result(pk);
    pk.Encrypt(result, to_ZZX(1) );
    for(int i=0; i<LEVEL; ++i)
    {
	    cout<<"before "<<i<<"-th multiply"<<endl;
	    result.multiplyBy(encU[i]);
	    cout<<"after  "<<i<<"-th multiply"<<"  >>>>>>>>>>>  ";
	    ZZX temp;sk.Decrypt(temp, result);cout<<temp[0]<<endl;
    }
    ZZX res;
    sk.Decrypt(res, result);
    cout<<"ciphertext result:= " << res[0]<<endl;
    
    long ans = 1;
    for(int i=0; i<LEVEL; ++i)     ans *= u[i];
    cout<<"plaintext answer:= " << ans  <<endl;
    cout<<"p^r = " << pow(p, r) <<endl;
    cout<<ans<<"%"<<pow(p,r)<<" = "<<ans%(int)pow(p,r)<<endl;

    return 0;
}
