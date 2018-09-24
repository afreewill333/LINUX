#include <cstddef>
#include <sys/time.h>
#include "FHE.h"
#include "EncryptedArray.h"
#include <NTL/ZZX.h>
#include <NTL/ZZ.h>
#include <gmp.h>

#define VEC_SIZE 144 // related to m,p,r; size of 145 would result in error.

NTL::ZZ  noPackingMultiplyAndSum(long u[], long v[], FHESecKey sk, FHEPubKey pk)
{
    // Vectors to hold the ciphertexts created from the elements of u and v
    std::vector<Ctxt> encU;
    std::vector<Ctxt> encV;

    // Each element is encrypted individually
    for(int i=0; i<VEC_SIZE; i++) {
	    Ctxt tempU(pk);
	    pk.Encrypt(tempU, to_ZZX(u[i]));
	    encU.push_back(tempU);

	    Ctxt tempV(pk);
	    pk.Encrypt(tempV, to_ZZX(v[i]));
	    encV.push_back(tempV);
    }

    std::cout << "HElib No packing encryption: " << "OK!" <<  std::endl;

    // Multiply the corresponding positions of vectors
    // and set the result in encU
    for(int i=0; i<VEC_SIZE; i++) {
	    encU[i] *= encV[i];
    }
    // Sum all the elements in encU
    // and save the result in the first position
    for(int i=1; i<VEC_SIZE; i++){
	    encU[0] += encU[i];
    }

    std::cout << "HElib No packing sum: " << "OK!"<<  std::endl;

    // Decrypt the first position of the vector,
    // which holds the value of the scalar product 
    ZZX result;
    sk.Decrypt(result, encU[0]); 

    std::cout << "HElib No packing decryption: " << "OK!" <<  std::endl;

    return result[0];
}

NTL::ZZ invertAndMultiply(long u[], long v[], FHESecKey sk, FHEPubKey pk)
{

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
    Ctxt encU(pk);                          
    Ctxt encV(pk);                          

    // Encrypt the polynomials into the ciphertexts
    pk.Encrypt(encU, U);
    pk.Encrypt(encV, V);

    std::cout << "HElib poly packing encryption: " << "OK!" <<  std::endl;

    // Multiply the ciphertexts and store the result into encU
    encU *= encV;

    std::cout << "HElib poly packing sum: " << "OK!" <<  std::endl;    

    // Decrypt the multiplied ciphertext into a polynomial using the secret key sk
    ZZX result;
    sk.Decrypt(result, encU);

    std::cout << "HElib poly packing decryption: " << "OK!" <<  std::endl;

    return result[VEC_SIZE - 1];
}

NTL::ZZ  multiplyAndTotalSum(long u[], long v[], FHEPubKey pk, FHESecKey sk, FHEcontext& context)
{
    // Creates a helper object based on the context
    EncryptedArray ea(context, context.alMod.getFactorsOverZZ()[0]); 

    // Create vectors from the values from the arrays.
    // The vectors should have the same size as the EncryptedArray (ea.size),
    // so fill the other positions with 0 which won't change the result
    std::vector<long> U(u, u + VEC_SIZE);
    std::vector<long> V(v, v + VEC_SIZE);

    for (int i = VEC_SIZE; i < ea.size(); i++) {
	U.push_back(0);
	V.push_back(0);
    }

    // Ciphertexts that will hold the encrypted vectors
    Ctxt encU(pk);
    Ctxt encV(pk);

    // Encrypt the whole vector into one ciphertext using packing
    ea.encrypt(encU, pk, U);
    ea.encrypt(encV, pk, V);

    std::cout << "HElib subfield packing encryption: " << "OK!" <<  std::endl;    

    // Multiply ciphertexts and set the result to encU
    encU.multiplyBy(encV);

    // Use the totalSums functions to sum all the elements
    // The result will have the sum in all positions of the vector
    totalSums(ea, encU);

    std::cout << "HElib subfield packing sum: " << "OK!" <<  std::endl;        
  
    // Decrypt the result(i.e., the scalar product value)
    ZZX result;
    sk.Decrypt(result, encU);

    std::cout << "HElib subfield packing decryption: " << "OK!" <<  std::endl;            

    return result[0];
}


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
  

    std::cout << "Finding m... " << std::flush;
    m = FindM(k, L, c, p, d, s, 0);           // Find a value for m given the specified values
    std::cout << "m = " << m << std::endl;

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

    // Initialize arrays
    // Array u will have even numbers, and array v will have odd numbers
    for (int i = 0; i < VEC_SIZE; i++) {
	u[i] = 2*i;
	v[i] = 2*i + 1;
    }
    std::cout << "Time taken for the initialization: " << "OK!" << std::endl;

    /*** METHOD 1: MULTIPLY AND SUM ARRAYS WITHOUT PACKING ***/
    ZZ method1Result = noPackingMultiplyAndSum(u, v, sk, pk);
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" <<std::endl;
    std::cout << "Multiply and sum arrays without packing method result: " << method1Result  <<std::endl;
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" <<std::endl;
    std::cout << "HElib without packing method done in " << "OK" <<  std::endl;	

    /*** METHOD 2: USE COEFFICIENT PACKING, INVERT AND MULTIPLY POLYNOMIALS ***/
    ZZ method2Result = invertAndMultiply(u, v, sk, pk);
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" <<std::endl;
    std::cout << "Invert and multiply method result: " << method2Result <<std::endl;
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" <<std::endl;
    std::cout << "HElib with polynomial packing method done in " << "OK!" <<  std::endl;

    /*** METHOD 3: USE SUBFIELDS PACKING, MULTIPLY AND SUM ***/
    ZZ method3Result = multiplyAndTotalSum(u, v, pk, sk, context);
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" <<std::endl;
    std::cout << "Multiply and totalSum method result: " << method3Result <<std::endl;
    std::cout << "HElib with subfield packing method done in " << "OK!" <<  std::endl;


    cout<< "TEST SUCCESSFULL" <<endl;

    return 0;

}
