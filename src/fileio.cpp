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
#include <stdexcept>

#include "fileio.h"

using namespace std;

fileio::fileOpenError::fileOpenError():
	runtime_error("Could not read file."){
}




void fileio::wavWrite(vector<double>sound,string filename){
	std::ofstream file(filename, ios::out|ios::binary|ios::trunc );
	if(file.is_open()){
		vector<int16_t> temp(sound.size());
		for(long int i=0;i<temp.size();i++){
			temp[i]=int(sound[i]*32767);
		}
		file.write((char*)&temp[0],temp.size()*sizeof(int16_t));
	}else{
		fileio::fileOpenError errorz = fileio::fileOpenError();
		throw(errorz);
	}
}

void fileio::append(vector<double>sound, string filename){
	if(sound.size() == 0){return;}
	std::ofstream file(filename, ios::out|ios::binary|ios::app);
	if(file.is_open()){
		vector<int16_t> temp(sound.size());
		for(long int i=0;i<temp.size();i++){
			temp[i]=int(sound[i]*32767);
		}
		file.write((char*)&temp[0],temp.size()*sizeof(int16_t));
	}else{
		fileio::fileOpenError errorz = fileio::fileOpenError();
		throw(errorz);
	}
}

vector<double> fileio::wavRead(string filename){
	ifstream file(filename, ios::in|ios::binary|ios::ate );
	if(file.is_open()){
		ifstream::pos_type length = file.tellg();
		file.seekg(0, ios::beg);
		vector<double> output(length/sizeof(int16_t));
		vector<int16_t> temp(length/sizeof(int16_t));
		file.read((char*)&temp[0], length);
		for(int i=0; i<temp.size(); i++){ output[i]=temp[i]/32767.; }
		return output;
	}else{
		fileio::fileOpenError errorz = fileio::fileOpenError();
		throw(errorz);
	}
}
