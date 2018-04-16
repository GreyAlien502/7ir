#include <fstream>
#include <assert.h>

#include "fileio.h"

using namespace std;

	static void writeLittleEndian(int value, std::ofstream& file, int size){
		assert(size<=sizeof(int));
		for( int i=0; i<size; i++ ){
			file.put(value >> 8*i);
		}
	}
void fileio::writeWavHeader(int sampleRate, std::ofstream& file){
	// RIFF chunk
	file.write("RIFF",4); // ChunkID
	file.write("1234",4); // ChunkSize placeholder
	file.write("WAVE",4); // Format

	// fmt  subchunk
	const int subchunk1Size = 16;
	const int audioFormat = 1;
	const int numChannels = 1;
	const int bitsPerSample = 16;
	const int byteRate = sampleRate * numChannels * bitsPerSample /8;
	const int blockAlign = numChannels * bitsPerSample/8;
	file.write("fmt ",4); // Subchunk1ID
	writeLittleEndian(subchunk1Size, file, 4);
	writeLittleEndian(audioFormat,   file, 2);
	writeLittleEndian(numChannels,   file, 2);
	writeLittleEndian(sampleRate,    file, 4);
	writeLittleEndian(byteRate,      file, 4);
	writeLittleEndian(blockAlign,    file, 2);
	writeLittleEndian(bitsPerSample, file, 2);

	// data subchunk
	file.write("data",4); // SubChunkID
	file.write("1234",4); // Subchunk2Size placeholder
}
void fileio::updateWavHeader(std::ofstream& file){
	file.seekp(0,ios::end);
	int fileSize = file.tellp();

	//write ChunkSize
	file.seekp(4);
	writeLittleEndian(fileSize-8, file, 4);

	//write ChunkSize
	file.seekp(40);
	writeLittleEndian(fileSize-40, file, 4);
}
void fileio::append(vector<float>sound, std::ofstream& file){
	if(sound.size()==0){return;}
	vector<int16_t> temp(sound.size());
	for(long int i=0;i<temp.size();i++){
		temp[i]=int(sound[i]*32767);
	}
	file.write((char*)&temp[0],temp.size()*sizeof(int16_t));
}

vector<float> fileio::wavRead(std::ifstream& file, int start, int end,std::ios_base::seekdir direction){
	assert(start>=0);
	assert(end>=0);
	const std::streampos HEADER_SIZE = 44;
	
	//get file size
	file.seekg(0, std::ios::end);
	int dataLength = file.tellg() - HEADER_SIZE;

	//cerr<<dataLength<<'	'<<start<<'	'<<end<<endl;

	int length;
	if(direction == std::ios::beg){
		length = (end -start)*sizeof(int16_t);
	}else if(direction == std::ios::end){
		length = dataLength + (-end -start)*sizeof(int16_t);
	}else{
		throw std::invalid_argument("Invalid direction");
	}

	if(length>dataLength){
		cerr<<endl<<dataLength<<"Warning: you're asking for too much."<<length<<endl;
		length = dataLength;
	}

	file.seekg(HEADER_SIZE+start*sizeof(int16_t), std::ios::beg);
	vector<float> output(length/sizeof(int16_t));
	vector<int16_t> temp(length/sizeof(int16_t));
	file.read((char*)&temp[0], length);
	for(int i=0; i<temp.size(); i++){ output[i]=temp[i]/32767.; }
	return output;
}
