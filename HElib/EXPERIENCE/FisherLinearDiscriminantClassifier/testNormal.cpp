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
	ifstream fin;
	fin.open("WDBC.csv");  // traindata

	//Step 0. Get data 
	vector<vector<double>> data;// only store the features without class
	vector<int> y;              // only store the class

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
		y.push_back( (diagnosis=="B"? +1:-1) );

		for(int i=0;i<30;i++)
		{
			getline(ss, cell, ',');

			double feature;
			stringstream sss;
			sss<<cell;
			sss>>feature;

			row[i] = feature;
		}
		data.push_back(row);
	}

	//Step 1. Separate data To 2 categories. 
	vector<vector<double>> dataB;
	vector<vector<double>> dataM;
	for(int i=0;i!=data.size();i++)
	{
		auto row = data[i];
		if(y[i]==+1)
			dataB.push_back(row);
		if(y[i]==-1)
			dataM.push_back(row);
	}

	//Step 2. Compute the Mean of Benign	
	vector<double> S_B(30);
	for(int i=0;i<30;i++) S_B[i] = 0;
	for(int i=0;i!=dataB.size();i++)
	{
		auto row = dataB[i];
		for(int j=0; j<row.size(); j++)
			S_B[j] += row[j];
	}
	vector<double> M_B(30);
	cout<<"M_B : "<<endl;
	for(int i=0;i<30;i++) 
	{
		M_B[i] = S_B[i]/dataB.size();
		cout<<M_B[i]<<endl;
	}
	cout<<endl;
	cout<<"dataB.size() = "<<dataB.size()<<" / "<<data.size()<<endl;
	
	//Step 3. Compute the Mean of Malignant
	vector<double> S_M(30);
	for(int i=0;i<30;i++) S_M[i] = 0;
	for(int i=0;i!=dataM.size();i++)
	{
		auto row = dataM[i];
		for(int j=0; j<row.size(); j++)
			S_M[j] += row[j];
	}
	vector<double> M_M(30);
	cout<<"M_M : "<<endl;
	for(int i=0;i<30;i++)
	{
		M_M[i] = S_M[i]/dataM.size();
		cout<<M_M[i]<<endl;
	}
	cout<<endl;
	cout<<"dataM.size() = "<<dataM.size()<<" / "<<data.size()<<endl;

	//Step 4. Compute the vector of Weight
	vector<double> w(30);
	for(int i=0;i<30;i++)
		w[i] = M_B[i] - M_M[i];

	//Step 5. Compute the mid-point of 2 Means.
	vector<double> x0(30);
	for(int i=0;i<30;i++)
		x0[i] = (M_B[i] + M_M[i])/2;

	//Step 6. Compute the offset of classifer
	double c = 0;
	for(int i=0;i<30;i++)
		c += w[i]*x0[i];
	cout<<"c = #"<<c<<"#"<<endl;

	// ***** classification stage ***** 

	// Given a new line(30)
	// f(x;w,c) = wTx -c   
	// IF f(x;w,c) > 0 : Benign ;
	// IF f(x;w,c) < 0 : Malignant;
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
		// SET  +1 - Benign ; -1 - Malignant 
		yy.push_back( (cell=="B"? +1:-1) );

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
		
		vector<double> line(30);
		for(int i=0;i<30;i++) line[i] = row[i];
		double result = 0;
		for(int i=0;i<30;i++)
			result += w[i]*line[i];
		result -= c;

		cout<<setw(4)<<i<<"\t";
		if(result>0) cout<<(( 1==yy[i])? "¡Ì" : "\t¡Á" )<<endl;
		else         cout<<((-1==yy[i])? "¡Ì" : "\t¡Á" )<<endl;

		if(result>0&&yy[i]>0) matchcount++;
		if(result<0&&yy[i]<0) matchcount++;
	}
	cout<<matchcount<<"/"<<testdata.size()<<" = "<<(double)matchcount/testdata.size()<<" is match rate."<<endl;

	fin.close();

	return 0;

}