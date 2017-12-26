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




void fileio::wavWrite(vector<float>sound,string filename){
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

void fileio::append(vector<float>sound, string filename){
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

vector<float> fileio::wavRead(string filename){
	ifstream file(filename, ios::in|ios::binary|ios::ate );
	if(file.is_open()){
		ifstream::pos_type length = file.tellg();
		file.seekg(0, ios::beg);
		vector<float> output(length/sizeof(int16_t));
		vector<int16_t> temp(length/sizeof(int16_t));
		file.read((char*)&temp[0], length);
		for(int i=0; i<temp.size(); i++){ output[i]=temp[i]/32767.; }
		return output;
	}else{
		fileio::fileOpenError errorz = fileio::fileOpenError();
		throw(errorz);
	}
}

//WRITE TO BINARY FILE
	template<typename type>
	void write_template(
		std::ostream& filestream,
		std::enable_if_t<std::is_fundamental<type>::value,type> scalar
	){
		filestream.write(reinterpret_cast<char*>(&scalar),sizeof(scalar)); 
	}
	template<typename type>
	void write_template(
		std::ostream& filestream,
		std::enable_if_t<!std::is_fundamental<type>::value,type> vec
	){
		write_template<int>(filestream,(int)vec.size());
		for(int index=0; index<vec.size(); index++){//TODO:bulk copy primitive type vectors?
			write_template<typename type::value_type>(filestream,vec[index]);
		}
	}
void fileio::write(std::ostream& filestream, int                   value){
	write_template<int>(filestream,value);
}
void fileio::write(std::ostream& filestream, float                value){
	write_template<float>(filestream,value);
}
void fileio::write(std::ostream& filestream, vector<float>        value){
	write_template<vector<float>>(filestream,value);
}
void fileio::write(std::ostream& filestream, vector<vector<float>>value){
	write_template<vector<vector<float>>>(filestream,value);
}

//READ FROM BINARY FILE
	template<typename type>
	std::enable_if_t<std::is_fundamental<type>::value,type>
	read_template(std::istream& filestream, type){
		type output;
		filestream.read(reinterpret_cast<char*>(&output),sizeof(output));
		return output;
	}

	template<typename type>
	std::vector<
		std::enable_if_t<
			std::is_fundamental< typename type::value_type >::value,
			typename type::value_type
		>
	> read_template(std::istream& filestream, type placeholder){
		using elementType = typename type::value_type;
		int size = read_template<int>(filestream,(int)0);
		vector<elementType> output = vector<elementType>(size);
		filestream.read(
			reinterpret_cast<char*>(&output[0]),
			size*sizeof(output[0])
		);
		return output;
	}

	template<typename type>
	vector<vector<typename type::value_type::value_type>> read_template(std::istream& filestream, type placeholder){
		using elementType = typename type::value_type::value_type;
		int size = read_template<int>(filestream,(int)0);
		vector<vector<elementType>> output = vector<vector<elementType>>(size);
		for(int index=0; index<size; index++){
			output[index] = read_template<vector<elementType>>(filestream,vector<elementType>());
		}
		return output;
	}
int fileio::read(std::istream& filestream,int placeholder){
	return read_template<int>(filestream,placeholder);
}
float fileio::read(std::istream& filestream,float placeholder){
	return read_template(filestream,placeholder);
}
std::vector<int> fileio::read(std::istream& filestream,std::vector<int> placeholder){
	return read_template<vector<int>>(filestream,placeholder);
}
std::vector<float> fileio::read(std::istream& filestream,std::vector<float> placeholder){
	return read_template<vector<float>>(filestream,placeholder);
}
std::vector<std::vector<float>> fileio::read(std::istream& filestream,std::vector<std::vector<float>>placeholder){
	return read_template<vector<vector<float>>>(filestream,placeholder);
}
