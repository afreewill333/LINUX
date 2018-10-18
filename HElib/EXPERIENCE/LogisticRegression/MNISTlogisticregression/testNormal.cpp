#include<cstdio>
#include<vector>
#include<string>
#include<fstream>
#include<iostream>
#include<iomanip>
#include<sstream>
#include<iterator>

using namespace std;

uint32_t swap_endian(uint32_t val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | (val >> 16);
}

int main(int argc, char *argv[])
{
	ifstream fin;
	fin.open("train-images.idx3-ubyte", ios::binary);  // traindata
	//it is import to write ifstream file(full_path, ios::binary); 
	//and not ifstream file(full_path);
	//https://stackoverflow.com/questions/8286668/how-to-read-mnist-data-in-c


	if(!fin.is_open())
	{
		cout<<"ERROR happens in opening file!"<<endl;
		exit(0);
	}

	uint32_t magic;
    uint32_t num_items;
    uint32_t rows;
    uint32_t cols;
	
	fin.read((char*)& magic, sizeof(magic));
	fin.read((char*)& num_items, sizeof(num_items));
	fin.read((char*)& rows, sizeof(rows));
	fin.read((char*)& cols, sizeof(cols));

	magic = swap_endian(magic);
    num_items = swap_endian(num_items);
    rows = swap_endian(rows);
    cols = swap_endian(cols);

	cout<<magic<<endl;
	cout<<num_items<<endl;
	cout<<rows<<endl;
	cout<<cols<<endl;

	vector<vector<uint8_t>> images;
	for(int k=0;k!=num_items;++k)
	{
		vector<uint8_t> image;
		for(int i=0;i!=rows;++i)
		{
			for(int j=0;j!=cols;++j)
			{
				uint8_t temp;
				fin.read((char*)& temp, sizeof(temp));
				image.push_back(temp);
				//printf("%02x", (unsigned int)temp);
			}
			//cout<<endl;
		}
		images.push_back(image);
	}

	// how to read a image
	auto image = images[num_items-1];
	for(int i=0;i!=rows;++i)
	{
		for(int j=0;j!=cols;++j)
		{
			printf("%02x", (unsigned int)image[i*rows+j]);
		}
		cout<<endl;
	}

	fin.close();
	fin.open("train-labels.idx1-ubyte", ios::binary); 

	if(!fin.is_open())
	{
		cout<<"ERROR happens in opening file!"<<endl;
		exit(0);
	}
	
	fin.read((char*)& magic, sizeof(magic));
	fin.read((char*)& num_items, sizeof(num_items));

	magic = swap_endian(magic);
    num_items = swap_endian(num_items);

	cout<<magic<<endl;
	cout<<num_items<<endl;

	vector<uint8_t> labels;
	for(int k=0;k!=num_items;++k)
	{
		uint8_t temp;
		fin.read((char*)& temp, sizeof(temp));
		labels.push_back(temp);
		//printf("%02x\n", (unsigned int)temp);
	}

	return 0;

}