#include<cstdio>
#include<string>
#include<fstream>
#include<iostream>
#include<iomanip>
#include<sstream>

using namespace std;

// double d =50.01234567891230;
// countfloat( d-(int)d );
int countfloat(double d)
{
	stringstream ss;
	ss.unsetf ( std::ios::floatfield );  // floatfield not set
	ss.precision(12);
	ss << d << '\n';

	string res;
	ss>>res;
	//cout<<"#"<<res<<"#"<<endl;
	
	return res.size() - res.find(".") - 1;
}

int main(int argc, char *argv[])
{
	//cout<<countfloat(234.4545)<<endl;

	ifstream fin;
	fin.open("WDBC.dat");

	ofstream fout;
	fout.open("result.csv", ios::out);

	string line;
	while(getline(fin, line))
	{
		string cell;
		stringstream ss(line);
		
		string ID;
		for(int i=0;i<32;i++)
		{
			getline(ss, cell, ',');
			cout<<setw(15)<<cell<<"\t\t"<<cell.size()-cell.find(".")-1;
			if(cell.size()-cell.find(".")-1>6)
				cout<<"<<<<<<<<<<<<<<<<<<<<<<";
			cout<<endl;
			fout<<cell<<",";
		}

		cout<<"++++++++++++++++++++++++++"<<endl;
		fout<<endl;

	}

	fout.close();

	fin.close();

	return 0;

}