#include<iostream>
#include<gmp.h>
#include<NTL/ZZ.h>

using namespace std;
using namespace NTL;

int main(int argc, char* argv[])
{
    //TEST GMP
    mpz_t t;
    mpz_init(t);
    mpz_ui_pow_ui(t,2,100);
    gmp_printf("2^100 = %Zd\n",t);

    mpz_clear(t);
   
    //TEST NTL
    ZZ a,b,c;
    cin>>a;
    cin>>b;
    c = (a+1)*(b+1);
  
    cout<<c<<endl;

    return 0;
}