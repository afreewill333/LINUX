#include<cstdio>
#include<cmath>
#include<vector>
#include<string>
#include<fstream>
#include<iostream>
#include<iomanip>
#include<sstream>
#include<iterator>
#include<random>
#include<set>

using namespace std;

double sigmoid(double x)
{
	//return 0.5 + 0.15*x  -0.0015*x*x*x; 
	return 1.0/(1 + exp(-x) );
}

int randint(int a, int b)
{
	static default_random_engine generator;
	uniform_int_distribution<int> distribution(a, b);

	return distribution(generator);
}

double innerProduct(vector<double> a, vector<double> b)
{
	if(a.size() != b.size())
	{
		cout<<"ERRORS HAPPEN IN innerProduct()"<<endl;
		exit(0);
	}

	double result = 0;
	for(int i=0;i!=a.size();++i)
		result += a[i]*b[i];

	return result;
}

vector<vector<double>> transpose(vector<vector<double>> mat)
{
	int M = mat.size();    // M rows
	int N = mat[0].size(); // N columns
	
	vector<vector<double>> res(N);
	for(int i=0;i!=res.size();++i) res[i].resize(M);

	for(int i=0;i!=res.size();++i)
		for(int j=0;j!=res[i].size();++j)
		res[i][j] = mat[j][i];

	return res;
}

int main(int argc, char *argv[])
{
	ifstream fin;
	fin.open("data_classification.csv");  // traindata

	//Step 0. Get data 
	vector<vector<double>> data;    // only store the features without class
	vector<int> label;              // only store the class

	string line;
	while(getline(fin, line))
	{
		vector<double> row(2);
		string cell;
		stringstream ss(line);
		int y;

		getline(ss, cell, ',');
		stringstream ss1;
		ss1<<cell;
		ss1>>row[0];
        
		getline(ss, cell, ',');
		stringstream ss2;
		ss2<<cell;
		ss2>>row[1];

		data.push_back(row);

		getline(ss, cell, ',');
		stringstream ss3;
		ss3<<cell;
		ss3>>y;

		label.push_back(y);
	}

	for(int i=0;i!=data.size();++i)
		cout<<setw(12)<<setprecision(12)<<data[i][0]<<setw(12)
		<<data[i][1]<<setw(12)<<label[i]<<endl;

	//Step 1. Use gradlent ascent to find the best parameters. 
	//        Turn data[0] to [1, data[0] ]
	//        vector<vector<double>> data;
	//        vector<int> label;
	for(int i=0;i!=data.size();++i)
		data[i].insert(data[i].begin(), 1.0);

	double alpha = 0.001; // step size
	int iters = 150;
	vector<double> weights;
	for(int i=0;i!=data[0].size();i++) weights.push_back(1.0);

	for(int j=0;j!=iters;++j)
	{
		vector<int> dataindex;
		for(int i=0;i!=data.size();++i)dataindex.push_back(i);
		
		for(int i=0;i!=data.size();++i)
		{
			alpha = 4/(1.0+j+i) +0.01;
			int randindex = randint(0,dataindex.size()-1);
			int indexindex = dataindex[randindex];
			double h = sigmoid(innerProduct(data[indexindex],weights));
			double error = label[indexindex] - h;

			for(int i=0;i!=weights.size();++i)
				weights[i] = weights[i] + alpha* error*data[indexindex][i];
			
			dataindex.erase(dataindex.begin()+randindex);	
		}
			
	}
	for(int i=0;i!=weights.size();++i)
		cout<<weights[i]<<endl;

	// ***** classification stage ***** 
	// Given weights and a new row
	// Step 1. row = [1, row]
	// Step 2. prob = sigmoid(innerProduct(row, weights))
	// Step 3. IF prob > 0.5: return 1.0 (Malignant)
	//         ELSE         : return 0.0 (Benign)
	ifstream ffin;
	ffin.open("data_classification.csv");
	vector<vector<double>> testdata;
	vector<int> yy;              
	while(getline(ffin, line))
	{
		vector<double> row(2);
		string cell;
		stringstream ss(line);
		int y;

		getline(ss, cell, ',');
		stringstream ss1;
		ss1<<cell;
		ss1>>row[0];
        
		getline(ss, cell, ',');
		stringstream ss2;
		ss2<<cell;
		ss2>>row[1];

		testdata.push_back(row);

		getline(ss, cell, ',');
		stringstream ss3;
		ss3<<cell;
		ss3>>y;

		yy.push_back(y);
	}
	ffin.close();
	int matchcount = 0;
	for(int i=0;i!=testdata.size();i++)
	{
		auto row = testdata[i];
		row.insert(row.begin(), 1.0); 
		
		double prob = sigmoid(innerProduct(row, weights));


		cout<<setw(4)<<i<<"\t";
		if(prob > 0.5) 
		{
			cout<<setw(7)<<prob<<(( 1==yy[i])? "\t¡Ì" : "\t¡Á" )<<endl;
			if(1==yy[i]) matchcount++;
		}
		else
		{			
			cout<<setw(7)<<prob<<(( 0==yy[i])? "\t¡Ì" : "\t¡Á" )<<endl;
			if(0==yy[i]) matchcount++;
		}
	}
	cout<<matchcount<<"/"<<testdata.size()<<" = "<<(double)matchcount/testdata.size()<<" is match rate."<<endl;

	fin.close();

	return 0;

}