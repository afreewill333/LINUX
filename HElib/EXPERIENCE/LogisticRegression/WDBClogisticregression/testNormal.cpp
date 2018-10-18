#include<cstdio>
#include<cmath>
#include<vector>
#include<string>
#include<fstream>
#include<iostream>
#include<iomanip>
#include<sstream>
#include<iterator>

using namespace std;

double sigmoid(double x)
{
	//return 0.5 + 0.15*x  -0.0015*x*x*x; 
	return 1.0/(1 + exp(-x) );
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
	fin.open("WDBC.csv");  // traindata

	//Step 0. Get data 
	vector<vector<double>> data;// only store the features without class
	vector<int> label;              // only store the class

	string line;
	while(getline(fin, line))
	{
		vector<double> row(30);

		string cell;
		stringstream ss(line);
		
		getline(ss, cell, ',');
		string ID = cell;      // ID store the 1st feature which is uselessfull.

		getline(ss, cell, ',');
		string diagnosis = cell;
		// SET  +1 - Benign ; -1 - Malignant 
		label.push_back( (diagnosis=="B"? 0:1) );

		for(int i=0;i<30;i++)
		{
			getline(ss, cell, ',');

			stringstream sss;
			sss<<cell;
			double feature;
			sss>>feature;

			row[i] = feature;
		}
		data.push_back(row);
	}

	//Step 1. Use gradlent ascent to find the best parameters. 
	//        Turn data[0] to [1, data[0] ]
	//        vector<vector<double>> data;
	//        vector<int> label;
	for(int i=0;i!=data.size();++i)
		data[i].insert(data[i].begin(), 1.0);

	double alpha = 0.001; // step size
	int iters = 500;
	vector<double> weights;
	for(int i=0;i!=data[0].size();i++) weights.push_back(0.0);

	auto dataT = transpose(data);
	for(int i=0;i!=iters;++i)
	{
		vector<double> h;
		for(int i=0;i!=data.size();++i)
		{
			auto row = data[i];
			double temp = innerProduct(row,weights);
			h.push_back( sigmoid(temp) );
		}

		vector<double> error;
		double curerr = 0.0;
		for(int i=0;i!=label.size();++i)
		{
			double err = label[i] - h[i];
			error.push_back(err);
			curerr += err*err;
		}
		if(curerr<0.5) break;

		for(int i=0;i!=weights.size();++i)
		{
			auto col = dataT[i];
			weights[i] = weights[i] + alpha* innerProduct(col, error);
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
	ffin.open("WDBC.dat");
	vector<vector<double>> testdata;
	vector<int> yy;              
	while(getline(ffin, line))
	{
		vector<double> row(30);

		string cell;
		stringstream ss(line);
		
		getline(ss, cell, ',');// ID
		getline(ss, cell, ',');// class
		// SET  0 - Benign ; 1 - Malignant 
		yy.push_back( (cell=="B"? 0 : 1) );

		for(int i=0;i<30;i++)
		{
			getline(ss, cell, ',');

			double feature;
			stringstream sss;
			sss<<cell;
			sss>>feature;
			row[i] = feature;		
		}
		testdata.push_back(row);
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
			cout<<1<<" -- "<<yy[i]<<"\t"
			    <<(( 1==yy[i])? "¡Ì" : "\t¡Á" )<<endl;
			if(1!=yy[i]) matchcount++;
		}
		else
		{			
			cout<<0<<" -- "<<yy[i]<<"\t"
			    <<(( 0==yy[i])? "¡Ì" : "\t¡Á" )<<endl;
			if(0!=yy[i]) matchcount++;
		}
	}
	cout<<matchcount<<"/"<<testdata.size()<<" = "<<(double)matchcount/testdata.size()<<" is match rate."<<endl;

	fin.close();

	return 0;

}