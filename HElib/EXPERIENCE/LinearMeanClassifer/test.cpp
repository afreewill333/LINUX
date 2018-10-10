#include<cstdio>
#include<vector>
#include<string>
#include<fstream>
#include<iostream>
#include<iomanip>
#include<sstream>
#include<iterator>

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

	//Step 0. Get data 
	vector<vector<double>> data;

	string line;
	while(getline(fin, line))
	{
		vector<double> row(31);

		string cell;
		stringstream ss(line);
		
		getline(ss, cell, ',');
		string ID = cell;

		getline(ss, cell, ',');
		string diagnosis = cell;
		// SET  0 - Benign ; 1 - Malignant 
		row[30] = (diagnosis=="B"? 0:1);

		for(int i=0;i<30;i++)
		{
			getline(ss, cell, ',');

			double feature;
			stringstream sss;
			sss<<cell;
			sss>>feature;
			row[i] = feature;
			
			cout<<setw(15)<<cell<<endl;
			fout<<cell<<",";
		}

		data.push_back(row);

		cout<<"++++++++++++++++++++++++++"<<endl;
		fout<<endl;
	}

	//for(auto it=data[568].begin();it!=data[568].end();++it)
	//	cout<<*it<<"\t";	

	//Step 1. Enlarge features to Integers
	//        Set factor = 1e7
	for(int i=0;i!=data.size();i++)
	{
		for(int j=0;j!=data[i].size()-1;j++)
			data[i][j] *= 1e7;
	}

	cout<<endl;
	for(int j=0;j!=data[568].size();j++)
		cout<<fixed<<data[568][j]<<"\t";
	cout<<endl;


	fout.close();

	fin.close();

	return 0;

}