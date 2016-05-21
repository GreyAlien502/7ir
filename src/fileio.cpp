#include <stdlib.h>
#include <stdio.h>
#include <cstdint>
#include <cmath>
#include <string.h>
#include <fftw3.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "fileio.h"

using namespace std;
bool fileio::save(vector<double>sound,char* filename){
	std::ofstream file(filename, ios::out|ios::binary|ios::trunc );
	if(file.is_open()){
		vector<int16_t> temp(sound.size());
		for(long unsigned int i=0;i<temp.size();i++){temp[i]=int(sound[i]*32767);}
		file.write((char*)&temp[0],temp.size()*sizeof(int16_t));
		return true;
	}else{
		return false;
	}
}

vector<double> fileio::read(const char* filename){
	ifstream file(filename, ios::in|ios::binary|ios::ate );
	if(file.is_open()){
		ifstream::pos_type length = file.tellg();
		file.seekg(0, ios::beg);
		vector<double> output(length/sizeof(int16_t));
		vector<int16_t> temp(length/sizeof(int16_t));
		file.read((char*)&temp[0], length);
		for(unsigned int i=0; i<temp.size(); i++){ output[i]=temp[i]/32767.; }
		return output;
	}else{
		cerr << "ERROR: error reading file.\n";
		exit(1);
	}
}
