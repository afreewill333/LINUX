#include<cstdio>
#include<string>
#include<fstream>
#include<iostream>
using namespace std;

int main(int argc, char *argv[])
{
	ifstream fin;
	fin.open("WDBC.dat");

	while(fin.good())
	{
		string str;
		getline(fin, str, ',');
		cout<<str<<endl;
	}

	fin.close();
	
	return 0;
}
