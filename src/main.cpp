#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <fftw3.h>
#include <fstream>
#include <iostream>

/*typedef struct{
  int length;
  char* content;
  }Lecian;*/


int save(char* array,int size, const char* filename){
	std::ofstream file;
	file.open(filename, std::ios::out|std::ios::binary|std::ios::trunc );
	if(file.is_open()){
		file.write(array,size);
		return true;
	}else{
		return false;
	}
}
char*  read(const char* filename,int * size){
	std::ifstream file;
	file.open(filename, std::ios::in|std::ios::binary|std::ios::ate );
	if(file.is_open()){
		std::ifstream::pos_type length = file.tellg();
		char* output = (char*) malloc(int(length));

		file.seekg(0, std::ios::beg);
		file.read(output,length);
		*size= int(length);
		return output;
	}else{
		size = 0;
		return 0;
	}
}

double* transpose(int factor, int length, double* sound, int N){
	double* output = (double*) malloc(sizeof(double)*length);
	double* in = (double*) malloc(sizeof(double)*N);
	fftw_complex* mid = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N/2+1);
	double* out = (double*) malloc(sizeof(double)*N);
	fftw_plan goThere= fftw_plan_dft_r2c_1d(N, in, mid, FFTW_ESTIMATE);
	fftw_plan goBaaq = fftw_plan_dft_c2r_1d(N, mid,out, FFTW_ESTIMATE);

	int start;
	int end;
	int step;
	if(factor<0){
		start = N/2+1-factor;
		end = 0;
		step = -1;
	}else{
		start = 0;
		end = N/2+1-factor;
		step = 1;
	}

		printf("bef\n");
	for(int pos=0;pos+N<length;pos+=N){
		memcpy(in,&sound[pos],N*sizeof(double));
		fftw_execute(goThere);
		for(int i=start; i!=end; i+=step){
			mid[i][0]=mid[i+factor][0];
			mid[i][1]=mid[i+factor][1];
		//	printf("h%d\n",mid[i]);
		}
		fftw_execute(goBaaq);
		memcpy(&output[pos],out,N*sizeof(double));
		//for(int i=0;i<N;i++){if(output[pos+i]!=out[i]){
	//		printf("%f\t%f\n",output[pos+i],out[i]);
	//	}}
	}
		printf("aef\n");
	for(int i=0;i<length;i++){
		output[i] /= N;
	}
	printf("uan\n");
	fftw_destroy_plan(goThere);
	printf("tuu\n");
	fftw_destroy_plan(goBaaq);
	printf("tri\n");
	fftw_free(mid);
	printf("fro\n");
	free(in);
	free(out);
	return output;
}




int main(){
	static const double PI = 3.14159265358979323;
	int sr = 44100;
	int size;
	//double* out = (double*) malloc(sizeof(double) * N);
		/*	sr*t;
		int C = 1055;
		int A = C*5/6;
		int F = C*4/6;
		int G = C*3/4;

		fftw_complex* mid = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N/2+1);
		
		fftw_plan plan     = fftw_plan_dft_r2c_1d(N, in, mid, FFTW_ESTIMATE);
		fftw_plan backplan = fftw_plan_dft_c2r_1d(N, mid,out, FFTW_ESTIMATE);*/


	int16_t* realin = (int16_t*) read("cown.wav",&size);
	int N =size/sizeof(int16_t);
	printf("%f\n",double(N)/sr);
	double* in = (double*) malloc(sizeof(double) * N);
	for(int i=0;i<N;i++){
		in[i] = double(realin[i]/32768.);
	}

		/*for(int i=0; i<N; i++){
			//n[i][0]=(double)(i*1*f%sr)/sr-.5;
			if(i<N*3){
				in[i][0] = .1*sin(i*2.*PI/sr*C);
				in[i][0]+= .1*sin(i*2.*PI/sr*C*5/8);
				in[i][0]+= .1*sin(i*2.*PI/sr*C*2);
			}else if(i<N/2){
				in[i][0] = .1*sin(i*2.*PI/sr*A);
				in[i][0]+= .1*sin(i*2.*PI/sr*A*5/8);
				in[i][0]+= .1*sin(i*2.*PI/sr*A*2);
			}else if(i<N*3/4){
				in[i][0] = .1*sin(i*2.*PI/sr*F);
				in[i][0]+= .1*sin(i*2.*PI/sr*F*3/5);
				in[i][0]+= .1*sin(i*2.*PI/sr*F*2);
			}else{
				in[i][0] = .1*sin(i*2.*PI/sr*G);
				in[i][0]+= .1*sin(i*2.*PI/sr*G*5/8);
				in[i][0]+= .1*sin(i*2.*PI/sr*G*2);
			}
			in[i][1]=0.;
		}

		fftw_execute(plan);
		for(int i= N/2+1;i>0;i--){
			if(i > (N/2+1)*.05){//her
				mid[i][0] =0.;// mid[i+1][0];
				mid[i][1] =0.;// mid[i+1][0];
			}else{
				mid[i][0] =  mid[i][0];
				mid[i][1] =  mid[i][1];
			}
			//double mag = sqrt(pow(mid[i][0],2) + pow(mid[i][1],2));
		}
		fftw_execute(backplan);

		for(int i=0;i<N;i++){
			out[i] /= N;
		}*/
	
	double* output = transpose(-100,N,in,sr/2);
	printf("ae\n");
	int16_t* realout = (int16_t*) malloc(size);
	for(int i=0;i<N;i++){
		realout[i  ] = int16_t(output[i]*32767/2);
		realout[N+i] = int16_t(realin[i]*32767/2);
	}

	if(save((char*) realout,size,"Zaundt")){
		printf("success\n");
	}else{
		printf("failure\n");
	}
	free(realout);
	free(realin);


	//fftw_destroy_plan(plan);
	//fftw_destroy_plan(backplan);
	free(in);
	//free(out);
	free(output);
}
