#include<iostream>
#include<cstdio>
#include "gmp.h"

using namespace std;

int main(int argc, char *argv[])
{
	mpz_t t;
	mpz_init(t);
	mpz_ui_pow_ui(t,2,100);
	
	gmp_printf("2^100=%Zd\n",t);

	mpz_clear(t);
	
	return 0;
}
