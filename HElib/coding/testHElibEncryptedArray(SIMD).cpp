#include "FHE.h"
#include "EncryptedArray.h"
#include <NTL/lzz_pXFactoring.h>
#include <fstream>
#include <sstream>
#include <sys/time.h>

int main(int argc, char **argv)
{
    /* On our trusted system we generate a new key
     * (or read one in) and encrypt the secret data set.
     */
    long m=0, p=100001651, r=2; // Native plaintext space (p should be prime)
                                // Computations will be 'modulo p'
    long L=16;          // Levels
    long c=3;           // Columns in key switching matrix
    long w=64;          // Hamming weight of secret key
    long d=0;
    long security = 128;

    ZZX G;
    m = FindM(security,L,c,p, d, 0, 0);
    FHEcontext context(m, p, r);
    buildModChain(context, L, c);
    FHESecKey secretKey(context);
    const FHEPubKey& publicKey = secretKey;

    //if(0 == d)
    G = context.alMod.getFactorsOverZZ()[0];

   secretKey.GenSecKey(w);

   addSome1DMatrices(secretKey);
   cout << "Generated key" << endl;

   EncryptedArray ea(context, G);

   long nslots = ea.size();

   vector<long> v1;
   for(int i = 0 ; i < nslots; i++) {
       v1.push_back(i*2);
   }
   Ctxt ct1(publicKey);
   ea.encrypt(ct1, publicKey, v1);
    
   vector<long> v2;
   Ctxt ct2(publicKey);
   for(int i = 0 ; i < nslots; i++) {
       v2.push_back(i*3);
   }
   ea.encrypt(ct2, publicKey, v2);
 
   // On the public (untrusted) system we
   // can now perform our computation
   Ctxt ctSum = ct1;
   Ctxt ctProd = ct1;

   ctSum += ct2;
   ctProd *= ct2;
 
    vector<long> res;
    ea.decrypt(ctSum, secretKey, res);

	ZZ pp, modulo; pp = p; modulo = 1;
	for(int i=0;i!=r;++i) modulo *= pp;
    cout << "All computations are modulo " << modulo << "." << endl;
    for(int i = 0; i < res.size(); i ++) {
        cout << v1[i] << " + " << v2[i] << " = " << v1[i] + v2[i] << " == " << res[i];
		if(v1[i] + v2[i] != res[i])
			cout<<" <<<<<<<<<<<<<<<<<<<<<<<";
		cout<< endl;
    }

    ea.decrypt(ctProd, secretKey, res);
    for(int i = 0; i < res.size(); i ++) {
        cout << v1[i] << " * " << v2[i] << " = " << v1[i] * v2[i] << " == " << res[i];
		if(v1[i] * v2[i] != res[i])
			cout<<" <<<<<<<<<<<<<<<<<<<<<<<";
		cout<< endl;
    }

    return 0;
}
