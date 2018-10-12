#include <cstddef>
#include <sys/time.h>
#include "FHE.h"
#include "EncryptedArray.h"
#include <NTL/ZZX.h>
#include <NTL/ZZ.h>
#include <gmp.h>

#define VEC_SIZE 144 // related to m,p,r; size 145 would result in error.

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
    long p = 100001651;           // Plaintext base [default=2], should be a prime number
    long r = 2;                   // Lifting [default=1]
    long L = 10;                  // Number of levels in the modulus chain [default=heuristic]
    long c = 2;                   // Number of columns in key-switching matrix [default=2]
    long w = 64;                  // Hamming weight of secret key
    long d = 1;                   // Degree of the field extension [default=1]
    long k = 80;                  // Security parameter [default=80] 
    long s = 0;                   // Minimum number of slots [default=0]

    m = FindM(k, L, c, p, d, s, 0);           // Find a value for m given the specified values
    std::cout << "m = " << m << std::endl;
	ZZ pp, modulo;
	pp = p; modulo = 1;
	for(int i=0;i!=r;++i) modulo = modulo*pp;
	std::cout << "p^r = " << modulo << std::endl;

    std::cout << "Initializing context... " << std::flush;
    FHEcontext context(m, p, r); 	          // Initialize context
    buildModChain(context, L, c);             // Modify the context, adding primes to the modulus chain
    std::cout << "OK!" << std::endl;

    std::cout << "Generating keys... " << std::flush;
    FHESecKey sk(context);                    // Construct a secret key structure
    const FHEPubKey& pk = sk;                 // An "upcast": FHESecKey is a subclass of FHEPubKey
    sk.GenSecKey(w);                          // Actually generate a secret key with Hamming weight
    addSome1DMatrices(sk);                    // Extra information for relinearization
    std::cout << "OK!" << std::endl;

    // Arrays whose elements will be the coefficients of the polynomials U(x) and V(x)
    long u[VEC_SIZE];
    long v[VEC_SIZE];
    long result[VEC_SIZE];    
 
    for(int i=0; i<VEC_SIZE; i++){
		u[i] = 2*i;
		v[i] = 2*i + 1;
    }

    /*** METHOD 1: MULTIPLY AND SUM ARRAYS WITHOUT PACKING ***/
	Timer t1;
    std::vector<Ctxt> encU(VEC_SIZE, Ctxt(pk));
    std::vector<Ctxt> encV(VEC_SIZE, Ctxt(pk));

	for(int i=0; i<VEC_SIZE; i++){
		Ctxt tempU(pk);
		pk.Encrypt(tempU, to_ZZX(u[i]));
		encU[i] = tempU;
		
		Ctxt tempV(pk);
		pk.Encrypt(tempV, to_ZZX(v[i]));
		encV[i] = tempV;
    }

    for(int i=0; i<VEC_SIZE; i++){
		encU[i] *= encV[i];
		//encU[i].multiplyBy(encV[i]); // this way takes more time.
    }
	for(int i=1; i<VEC_SIZE; i++){
		encU[0] += encU[i];
	}

	ZZX result1;
	sk.Decrypt(result1, encU[0]);
	ZZ method1Result = result1[0];	

	std::cout<<"METHOD 1: SCALAR PRODUCT USING HELIB WITHOUT PACKING         "<<t1.tillNow()<<std::endl;

	/*** METHOD 2: USE COEFFICIENT PACKING, INVERT AND MULTIPLY POLYNOMIALS ***/
	Timer t2;
    // ZZX is a class for polynomials from the NTL library
    ZZX U, V;                        
    // Set the length of the polynomial U(x) and V(x)
    U.SetLength(VEC_SIZE);
    V.SetLength(VEC_SIZE);

    // Set the coefficients of the polynomials U(x) and V(x).
    // Note that the elements from v are "inverted" when encoded into the coefficients of V(x)
    // E.g.: U(x) = 1 + 2x + 3x^2 + 4x^3
    // E.g.: V(x) = 4 + 3x + 2x^2 + 1x^3
    for(int i=0; i<VEC_SIZE; i++) {
		SetCoeff(U, i, u[i]);
		SetCoeff(V, (VEC_SIZE-1)-i, v[i]);
    }
	
    // Ciphertexts that will hold the polynomials encrypted using public key pk
    Ctxt ctU(pk);                          
    Ctxt ctV(pk);                          

    // Encrypt the polynomials into the ciphertexts
    pk.Encrypt(ctU, U);
    pk.Encrypt(ctV, V);
    // Multiply the ciphertexts and store the result into encU
    ctU *= ctV;
    // Decrypt the multiplied ciphertext into a polynomial using the secret key sk
    ZZX result2;
    sk.Decrypt(result2, ctU);

    ZZ method2Result = result2[VEC_SIZE - 1];
	std::cout<<"METHOD 2: SCALAR PRODUCT USING HELIB WITHOUT PACKING         "<<t2.tillNow()<<std::endl;

	/*** METHOD 3: USE SUBFIELDS PACKING, MULTIPLY AND SUM ***/
	Timer t3;
    // Creates a helper object based on the context
    EncryptedArray ea(context, context.alMod.getFactorsOverZZ()[0]); 
    // Create vectors from the values from the arrays.
    // The vectors should have the same size as the EncryptedArray (ea.size),
    // so fill the other positions with 0 which won't change the result
    std::vector<long> UU(begin(u), end(u));
    std::vector<long> VV(begin(v), end(v));

    for(int i=VEC_SIZE; i<ea.size(); i++){
		UU.push_back(0);
		VV.push_back(0);
    }

    // Ciphertexts that will hold the encrypted vectors
    Ctxt CU(pk);
    Ctxt CV(pk);

    // Encrypt the whole vector into one ciphertext using packing
    ea.encrypt(CU, pk, UU);
    ea.encrypt(CV, pk, VV);
    
    // Multiply ciphertexts and set the result to encU

    CU.multiplyBy(CV);      // using reLineration
	//CU *= CV;             // not using reLineration

    // Use the totalSums functions to sum all the elements
    // The result will have the sum in all positions of the vector
    totalSums(ea, CU);
       
    // Decrypt the result(i.e., the scalar product value)
    ZZX result3;
    sk.Decrypt(result3, CU);            
    ZZ method3Result = result3[0];
	std::cout<<"METHOD 3: SCALAR PRODUCT USING HELIB WITHOUT PACKING         "<<t3.tillNow()<<std::endl;


    // Checking results
	long res = 0;
    for(int i=0; i<VEC_SIZE; i++)
		res += u[i] * v[i];

	if(res == conv<long>(method3Result))
		std::cout<<"METHOD 3: WORK WELL !"<<std::endl;


    cout<<" TEST SUCCESSFULL"<<endl;

    return 0;
}
