#include<cstdio>
#include<string>
#include<fstream>
#include<iostream>
#include<sstream>

using namespace std;

int main(int argc, char *argv[])
{
	ifstream fin;
	fin.open("WDBC.dat");

	ofstream fout;
	fout.open("result.csv", ios::out);

	string line;
	while(getline(fin, line))
	{
		string cell;
		stringstream ss(line);
		
		for(int i=0;i<32;i++)
		{
			getline(ss, cell, ',');
			cout<<cell<<endl;
			fout<<cell<<",";
		}

		cout<<"++++++++++++++++++++++++++"<<endl;
		fout<<endl;

	}

	fout.close();

	fin.close();

	return 0;

}