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

int main(int argc, char **argv)
{

    /*** BEGIN INITIALIZATION ***/
    long m = 0;                   // Specific modulus
    long p = 100001651;           // Plaintext base [default=2], should be a prime number
    long r = 1;                   // Lifting [default=1]
   
    long L = 15;                  // Number of levels in the modulus chain [default=heuristic]
    long c = 3;                   // Number of columns in key-switching matrix [default=2]
    long w= 64;                  // Hamming weight of secret key
    long d = 1;                   // Degree of the field extension [default=1]
    long k = 80;                  // Security parameter [default=80] 
    long s = 0;                   // Minimum number of slots [default=0]

    m = FindM(k, L, c, p, d, s, 0);         // to choose all the parameters ?
    FHEcontext context(m, p, r); 	    // Initialize context
    buildModChain(context, L, c);           // Modify the contex,
    FHESecKey sk(context);                  // Construct a secret key structure
    const FHEPubKey& pk = sk;               // FHESecKey is a subclass of FHEPubKey
    sk.GenSecKey(w);                        // Actually generate a secret key
    addSome1DMatrices(sk);                  // compute key-switching matrices that relinearization needs.

    // constuct an Encrypted array object ea that is associated with the given context.
    EncryptedArray ea(context, context.alMod.getFactorsOverZZ()[0]); 
    cout<<"EncryptedArray ea.size() = "<<ea.size()<<endl;

    long nslots = ea.size();                // number of plaintext slots
    // PlaintextArray objects associated with the given EncryptedArray ea
    NewPlaintextArray p0(ea);
    NewPlaintextArray p1(ea);
    NewPlaintextArray p2(ea);
    NewPlaintextArray p3(ea);
    // generate random plaintexts: slots initalized with random elements of Z[x]/(G,p^r)
    random(ea, p0);
    random(ea, p1);
    random(ea, p2);
    random(ea, p3);
    // construct ciphertexts associated with the given public key
    Ctxt c0(pk), c1(pk), c2(pk), c3(pk);

    cout<< "p0 = "<< p0 <<endl;

    // encrypt each PlaintextArray
    ea.encrypt(c0, pk, p0);
    ea.encrypt(c1, pk, p1);
    ea.encrypt(c2, pk, p2);
    ea.encrypt(c3, pk, p3);

    // two random constants
    NewPlaintextArray const1(ea);
    NewPlaintextArray const2(ea);
    random(ea, const1);
    random(ea, const2);

    // Perform some simple computations directly on the plaintext arrays:
    mul(ea, p1, p0);  // p1 = p1 * p0 (slot-wise modulo G)
    add(ea, p0, const1); // p0 = p0 + const1
    mul(ea, p2, const2); // p2 = p2 * const2
    
    NewPlaintextArray tmp_p(p1); // tmp = p1
    // shift-amout: random number in [-nslots/2..nslots.2]
    long shamt = RandomBnd(2*(nslots/2) +1) - (nslots/2);
    shift(ea, tmp_p, shamt); // shift tmp_p by shamt
    add(ea, p2, tmp_p);  // p2 = p2 + tmp_p
    // rotation-amout: random number in [-(nslots-1)..nslots-1]
    long rotamt = RandomBnd(2*nslots -1) - (nslots -1);
    rotate(ea, p2, rotamt);  // rotate p2 by rotamt

    // Use :: to call global variable, which is probably hidden in the .h files.
    // Test :: asdfghjkl added in the EncryptedArray.h
    cout<< ::asdfghjkl <<endl;
    ::negate(ea, p1); // p1 = -p1
    mul(ea, p3, p2);  // p3 = p3 * p2
    sub(ea, p0, p3);  // p0 = p0 - p3
    
    // Perform the  same operations on the ciphertexts
    ZZX const1_poly, const2_poly;
    ea.encode(const1_poly, const1);  // encode const1 as plaintext polynomial
    ea.encode(const2_poly, const2);  // encode const2 as plaintext polynomial
    
    c1.multiplyBy(c0);  // c1 = c1 * c0
    c0.addConstant(const1_poly); // c0 = c0 + const1
    c2.multByConstant(const2_poly); // c2 = c2 * const2

    Ctxt tmp(c1); // tmp = c1
    ea.shift(tmp, shamt); // shift tmp by shamt
    c2 += tmp; // c2 = c2 + tmp
    ea.rotate(c2, rotamt); // rotate c2 by shamt
   
    c1.negate(); // c1 = -c1
    c3.multiplyBy(c2); // c3 = c3 * c2
    c0 -= c3;  // c0 = c0 - c3

    c0.cleanUp();
    c1.cleanUp();
    c2.cleanUp();
    c3.cleanUp();

    // Decrypt the ciphertexts and compare
    NewPlaintextArray pp0(ea);
    NewPlaintextArray pp1(ea);
    NewPlaintextArray pp2(ea);
    NewPlaintextArray pp3(ea);

    ea.decrypt(c0, sk, pp0);
    ea.decrypt(c1, sk, pp1);
    ea.decrypt(c2, sk, pp2);
    ea.decrypt(c3, sk, pp3);

    if(!equals(ea, pp0, p0)) cerr<<"oops 0"<<endl;
    if(!equals(ea, pp1, p1)) cerr<<"oops 1"<<endl;
    if(!equals(ea, pp2, p2)) cerr<<"oops 2"<<endl;
    if(!equals(ea, pp3, p3)) cerr<<"oops 3"<<endl;

    cout<<" test is done."<<endl;
   
    if( equals(ea, pp0, p0) && equals(ea, pp1, p1) && equals(ea, pp2, p2) && equals(ea, pp3, p3) )
	    cout<<"GOOD"<<endl;
    else
	    cout<<"BAD"<<endl;

    return 0;

}
