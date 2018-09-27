#include <cstddef>
#include <ctime>
#include "FHE.h"
#include "EncryptedArray.h"
#include <NTL/ZZX.h>
#include <NTL/ZZ.h>
#include <gmp.h>
#include <iterator>

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

#define VEC_SIZE 144 // related to m,p,r; size of 145 would result in error.

int main(int argc, char **argv)
{

    /*** BEGIN INITIALIZATION ***/
    long m = 0;                   // Specific modulus
    long p = 100001651;           // Plaintext base [default=2], should be a prime number
    long r = 1;                   // Lifting [default=1]
    long L = 10;                  // Number of levels in the modulus chain [default=heuristic]
    long c = 2;                   // Number of columns in key-switching matrix [default=2]
    long w = 64;                  // Hamming weight of secret key
    long d = 1;                   // Degree of the field extension [default=1]
    long k = 80;                  // Security parameter [default=80] 
    long s = 0;                   // Minimum number of slots [default=0]

    m = FindM(k, L, c, p, d, s, 0);        
    std::cout << "m = " << m << std::endl;
    FHEcontext context(m, p, r); 	          // Initialize context
    buildModChain(context, L, c);             // Modify the context, adding primes to the modulus chain
    FHESecKey sk(context);                    // Construct a secret key structure
    const FHEPubKey& pk = sk;                 // An "upcast": FHESecKey is a subclass of FHEPubKey
    sk.GenSecKey(w);                          // Actually generate a secret key with Hamming weight
    addSome1DMatrices(sk);                    // Extra information for relinearization

    // Arrays whose elements will be the coefficients of the polynomials U(x) and V(x)
    long u[VEC_SIZE];
    long v[VEC_SIZE];   
    for (int i = 0; i < VEC_SIZE; i++) {
		u[i] = 2*i;
		v[i] = 2*i + 1;
    }


    /********************** USE SUBFIELDS PACKING, MULTIPLY AND SUM **********************/
    // Creates a helper object based on the context
    EncryptedArray ea(context, context.alMod.getFactorsOverZZ()[0]); 
    cout<<"EncryptedArray ea.size() = "<<ea.size()<<std::endl;
    std::vector<long> U(begin(u), end(u));
    std::vector<long> V(begin(v), end(v));

    for (int i = VEC_SIZE; i < ea.size(); i++) {
		U.push_back(0);
		V.push_back(0);
    }

    // Ciphertexts that will hold the encrypted vectors
    Ctxt encU(pk);
    Ctxt encV(pk);

    Timer t1;
    // Encrypt the whole vector into one ciphertext using packing
    ea.encrypt(encU, pk, U);
    ea.encrypt(encV, pk, V);
    std::cout << "HElib subfield packing encryption: OK! " <<t1.tillNow()<< std::endl; 

    Timer t2;
    // Multiply ciphertexts and set the result to encU
    encU.multiplyBy(encV);
    std::cout << "HElib subfield packing multiply : OK! "<<t2.tillNow()<< std::endl;

    // Use the totalSums functions to sum all the elements
    // The result will have the sum in all positions of the vector
    totalSums(ea, encU);
    std::cout << "HElib subfield packing sum: OK!" <<  std::endl;   
  
    // Decrypt the result(i.e., the scalar product value)
    ZZX result;
    sk.Decrypt(result, encU);     

    ZZ method3Result = result[0];
    std::cout << "HElib with subfield packing method Multiply and totalSum method result: " << method3Result <<std::endl;
 

    return 0;
}
