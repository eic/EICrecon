//Code generated automatically by TMVA for Inference of Model file [trainedTaggerRegressionModel.h5] at [Mon Jun  5 08:11:01 2023] 

#ifndef TMVA_SOFIE_TRAINEDTAGGERREGRESSIONMODEL
#define TMVA_SOFIE_TRAINEDTAGGERREGRESSIONMODEL

#include<algorithm>
#include<cmath>
#include<vector>
#include "SOFIE_common.h"
#include <fstream>

namespace TMVA_SOFIE_trainedTaggerRegressionModel{
namespace BLAS{
	extern "C" void sgemv_(const char * trans, const int * m, const int * n, const float * alpha, const float * A,
	                       const int * lda, const float * X, const int * incx, const float * beta, const float * Y, const int * incy);
	extern "C" void sgemm_(const char * transa, const char * transb, const int * m, const int * n, const int * k,
	                       const float * alpha, const float * A, const int * lda, const float * B, const int * ldb,
	                       const float * beta, float * C, const int * ldc);
}//BLAS
struct Session {
std::vector<float> fTensor_dense4bias0 = std::vector<float>(3);
float * tensor_dense4bias0 = fTensor_dense4bias0.data();
std::vector<float> fTensor_dense3bias0 = std::vector<float>(32);
float * tensor_dense3bias0 = fTensor_dense3bias0.data();
std::vector<float> fTensor_dense3kernel0 = std::vector<float>(2048);
float * tensor_dense3kernel0 = fTensor_dense3kernel0.data();
std::vector<float> fTensor_dense2bias0 = std::vector<float>(64);
float * tensor_dense2bias0 = fTensor_dense2bias0.data();
std::vector<float> fTensor_dense4kernel0 = std::vector<float>(96);
float * tensor_dense4kernel0 = fTensor_dense4kernel0.data();
std::vector<float> fTensor_dense2kernel0 = std::vector<float>(8192);
float * tensor_dense2kernel0 = fTensor_dense2kernel0.data();
std::vector<float> fTensor_dense1bias0 = std::vector<float>(128);
float * tensor_dense1bias0 = fTensor_dense1bias0.data();
std::vector<float> fTensor_dense1kernel0 = std::vector<float>(131072);
float * tensor_dense1kernel0 = fTensor_dense1kernel0.data();
std::vector<float> fTensor_densebias0 = std::vector<float>(1024);
float * tensor_densebias0 = fTensor_densebias0.data();
std::vector<float> fTensor_densekernel0 = std::vector<float>(4096);
float * tensor_densekernel0 = fTensor_densekernel0.data();
std::vector<float> fTensor_densebias0bcast = std::vector<float>(1024);
float * tensor_densebias0bcast = fTensor_densebias0bcast.data();
std::vector<float> fTensor_dense2bias0bcast = std::vector<float>(64);
float * tensor_dense2bias0bcast = fTensor_dense2bias0bcast.data();
std::vector<float> fTensor_denseDense = std::vector<float>(1024);
float * tensor_denseDense = fTensor_denseDense.data();
std::vector<float> fTensor_dense1Tanh0 = std::vector<float>(128);
float * tensor_dense1Tanh0 = fTensor_dense1Tanh0.data();
std::vector<float> fTensor_denseTanh0 = std::vector<float>(1024);
float * tensor_denseTanh0 = fTensor_denseTanh0.data();
std::vector<float> fTensor_dense1bias0bcast = std::vector<float>(128);
float * tensor_dense1bias0bcast = fTensor_dense1bias0bcast.data();
std::vector<float> fTensor_dense1Dense = std::vector<float>(128);
float * tensor_dense1Dense = fTensor_dense1Dense.data();
std::vector<float> fTensor_dense4BiasAdd0 = std::vector<float>(3);
float * tensor_dense4BiasAdd0 = fTensor_dense4BiasAdd0.data();
std::vector<float> fTensor_dense2Dense = std::vector<float>(64);
float * tensor_dense2Dense = fTensor_dense2Dense.data();
std::vector<float> fTensor_dense3bias0bcast = std::vector<float>(32);
float * tensor_dense3bias0bcast = fTensor_dense3bias0bcast.data();
std::vector<float> fTensor_dense3Dense = std::vector<float>(32);
float * tensor_dense3Dense = fTensor_dense3Dense.data();
std::vector<float> fTensor_dense3Tanh0 = std::vector<float>(32);
float * tensor_dense3Tanh0 = fTensor_dense3Tanh0.data();
std::vector<float> fTensor_dense2Tanh0 = std::vector<float>(64);
float * tensor_dense2Tanh0 = fTensor_dense2Tanh0.data();
std::vector<float> fTensor_dense4bias0bcast = std::vector<float>(3);
float * tensor_dense4bias0bcast = fTensor_dense4bias0bcast.data();


Session(std::string filename ="") {
  std::cout << filename << std::endl;
   if (filename.empty()) filename = "trainedTaggerRegressionModel.dat";
   std::cout << filename << std::endl;
   std::ifstream f;
   f.open(filename);
   if (!f.is_open()){
      throw std::runtime_error("tmva-sofie failed to open file for input weights");
   }
   std::string tensor_name;
   int length;
   f >> tensor_name >> length;
   if (tensor_name != "tensor_dense4bias0" ) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor name; expected name is tensor_dense4bias0 , read " + tensor_name;
      throw std::runtime_error(err_msg);
    }
   if (length != 3) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor size; expected size is 3 , read " + std::to_string(length) ;
      throw std::runtime_error(err_msg);
    }
    for (int i =0; i < length; ++i) 
       f >> tensor_dense4bias0[i];
   f >> tensor_name >> length;
   if (tensor_name != "tensor_dense3bias0" ) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor name; expected name is tensor_dense3bias0 , read " + tensor_name;
      throw std::runtime_error(err_msg);
    }
   if (length != 32) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor size; expected size is 32 , read " + std::to_string(length) ;
      throw std::runtime_error(err_msg);
    }
    for (int i =0; i < length; ++i) 
       f >> tensor_dense3bias0[i];
   f >> tensor_name >> length;
   if (tensor_name != "tensor_dense3kernel0" ) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor name; expected name is tensor_dense3kernel0 , read " + tensor_name;
      throw std::runtime_error(err_msg);
    }
   if (length != 2048) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor size; expected size is 2048 , read " + std::to_string(length) ;
      throw std::runtime_error(err_msg);
    }
    for (int i =0; i < length; ++i) 
       f >> tensor_dense3kernel0[i];
   f >> tensor_name >> length;
   if (tensor_name != "tensor_dense2bias0" ) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor name; expected name is tensor_dense2bias0 , read " + tensor_name;
      throw std::runtime_error(err_msg);
    }
   if (length != 64) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor size; expected size is 64 , read " + std::to_string(length) ;
      throw std::runtime_error(err_msg);
    }
    for (int i =0; i < length; ++i) 
       f >> tensor_dense2bias0[i];
   f >> tensor_name >> length;
   if (tensor_name != "tensor_dense4kernel0" ) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor name; expected name is tensor_dense4kernel0 , read " + tensor_name;
      throw std::runtime_error(err_msg);
    }
   if (length != 96) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor size; expected size is 96 , read " + std::to_string(length) ;
      throw std::runtime_error(err_msg);
    }
    for (int i =0; i < length; ++i) 
       f >> tensor_dense4kernel0[i];
   f >> tensor_name >> length;
   if (tensor_name != "tensor_dense2kernel0" ) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor name; expected name is tensor_dense2kernel0 , read " + tensor_name;
      throw std::runtime_error(err_msg);
    }
   if (length != 8192) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor size; expected size is 8192 , read " + std::to_string(length) ;
      throw std::runtime_error(err_msg);
    }
    for (int i =0; i < length; ++i) 
       f >> tensor_dense2kernel0[i];
   f >> tensor_name >> length;
   if (tensor_name != "tensor_dense1bias0" ) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor name; expected name is tensor_dense1bias0 , read " + tensor_name;
      throw std::runtime_error(err_msg);
    }
   if (length != 128) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor size; expected size is 128 , read " + std::to_string(length) ;
      throw std::runtime_error(err_msg);
    }
    for (int i =0; i < length; ++i) 
       f >> tensor_dense1bias0[i];
   f >> tensor_name >> length;
   if (tensor_name != "tensor_dense1kernel0" ) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor name; expected name is tensor_dense1kernel0 , read " + tensor_name;
      throw std::runtime_error(err_msg);
    }
   if (length != 131072) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor size; expected size is 131072 , read " + std::to_string(length) ;
      throw std::runtime_error(err_msg);
    }
    for (int i =0; i < length; ++i) 
       f >> tensor_dense1kernel0[i];
   f >> tensor_name >> length;
   if (tensor_name != "tensor_densebias0" ) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor name; expected name is tensor_densebias0 , read " + tensor_name;
      throw std::runtime_error(err_msg);
    }
   if (length != 1024) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor size; expected size is 1024 , read " + std::to_string(length) ;
      throw std::runtime_error(err_msg);
    }
    for (int i =0; i < length; ++i) 
       f >> tensor_densebias0[i];
   f >> tensor_name >> length;
   if (tensor_name != "tensor_densekernel0" ) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor name; expected name is tensor_densekernel0 , read " + tensor_name;
      throw std::runtime_error(err_msg);
    }
   if (length != 4096) {
      std::string err_msg = "TMVA-SOFIE failed to read the correct tensor size; expected size is 4096 , read " + std::to_string(length) ;
      throw std::runtime_error(err_msg);
    }
    for (int i =0; i < length; ++i) 
       f >> tensor_densekernel0[i];
   f.close();
   {
      float * data = TMVA::Experimental::SOFIE::UTILITY::UnidirectionalBroadcast<float>(tensor_densebias0,{ 1024 }, { 1 , 1024 });
      std::copy(data, data + 1024, tensor_densebias0bcast);
      delete [] data;
   }
   {
      float * data = TMVA::Experimental::SOFIE::UTILITY::UnidirectionalBroadcast<float>(tensor_dense1bias0,{ 128 }, { 1 , 128 });
      std::copy(data, data + 128, tensor_dense1bias0bcast);
      delete [] data;
   }
   {
      float * data = TMVA::Experimental::SOFIE::UTILITY::UnidirectionalBroadcast<float>(tensor_dense2bias0,{ 64 }, { 1 , 64 });
      std::copy(data, data + 64, tensor_dense2bias0bcast);
      delete [] data;
   }
   {
      float * data = TMVA::Experimental::SOFIE::UTILITY::UnidirectionalBroadcast<float>(tensor_dense3bias0,{ 32 }, { 1 , 32 });
      std::copy(data, data + 32, tensor_dense3bias0bcast);
      delete [] data;
   }
   {
      float * data = TMVA::Experimental::SOFIE::UTILITY::UnidirectionalBroadcast<float>(tensor_dense4bias0,{ 3 }, { 1 , 3 });
      std::copy(data, data + 3, tensor_dense4bias0bcast);
      delete [] data;
   }
}

std::vector<float> infer(float* tensor_denseinput){

//--------- Gemm
   char op_0_transA = 'n';
   char op_0_transB = 'n';
   int op_0_m = 1;
   int op_0_n = 1024;
   int op_0_k = 4;
   float op_0_alpha = 1;
   float op_0_beta = 1;
   int op_0_lda = 4;
   int op_0_ldb = 1024;
   std::copy(tensor_densebias0bcast, tensor_densebias0bcast + 1024, tensor_denseDense);
   BLAS::sgemm_(&op_0_transB, &op_0_transA, &op_0_n, &op_0_m, &op_0_k, &op_0_alpha, tensor_densekernel0, &op_0_ldb, tensor_denseinput, &op_0_lda, &op_0_beta, tensor_denseDense, &op_0_n);

//------ TANH
   for (int id = 0; id < 1024 ; id++){
      tensor_denseTanh0[id] = std::tanh(tensor_denseDense[id]);
   }

//--------- Gemm
   char op_2_transA = 'n';
   char op_2_transB = 'n';
   int op_2_m = 1;
   int op_2_n = 128;
   int op_2_k = 1024;
   float op_2_alpha = 1;
   float op_2_beta = 1;
   int op_2_lda = 1024;
   int op_2_ldb = 128;
   std::copy(tensor_dense1bias0bcast, tensor_dense1bias0bcast + 128, tensor_dense1Dense);
   BLAS::sgemm_(&op_2_transB, &op_2_transA, &op_2_n, &op_2_m, &op_2_k, &op_2_alpha, tensor_dense1kernel0, &op_2_ldb, tensor_denseTanh0, &op_2_lda, &op_2_beta, tensor_dense1Dense, &op_2_n);

//------ TANH
   for (int id = 0; id < 128 ; id++){
      tensor_dense1Tanh0[id] = std::tanh(tensor_dense1Dense[id]);
   }

//--------- Gemm
   char op_4_transA = 'n';
   char op_4_transB = 'n';
   int op_4_m = 1;
   int op_4_n = 64;
   int op_4_k = 128;
   float op_4_alpha = 1;
   float op_4_beta = 1;
   int op_4_lda = 128;
   int op_4_ldb = 64;
   std::copy(tensor_dense2bias0bcast, tensor_dense2bias0bcast + 64, tensor_dense2Dense);
   BLAS::sgemm_(&op_4_transB, &op_4_transA, &op_4_n, &op_4_m, &op_4_k, &op_4_alpha, tensor_dense2kernel0, &op_4_ldb, tensor_dense1Tanh0, &op_4_lda, &op_4_beta, tensor_dense2Dense, &op_4_n);

//------ TANH
   for (int id = 0; id < 64 ; id++){
      tensor_dense2Tanh0[id] = std::tanh(tensor_dense2Dense[id]);
   }

//--------- Gemm
   char op_6_transA = 'n';
   char op_6_transB = 'n';
   int op_6_m = 1;
   int op_6_n = 32;
   int op_6_k = 64;
   float op_6_alpha = 1;
   float op_6_beta = 1;
   int op_6_lda = 64;
   int op_6_ldb = 32;
   std::copy(tensor_dense3bias0bcast, tensor_dense3bias0bcast + 32, tensor_dense3Dense);
   BLAS::sgemm_(&op_6_transB, &op_6_transA, &op_6_n, &op_6_m, &op_6_k, &op_6_alpha, tensor_dense3kernel0, &op_6_ldb, tensor_dense2Tanh0, &op_6_lda, &op_6_beta, tensor_dense3Dense, &op_6_n);

//------ TANH
   for (int id = 0; id < 32 ; id++){
      tensor_dense3Tanh0[id] = std::tanh(tensor_dense3Dense[id]);
   }

//--------- Gemm
   char op_8_transA = 'n';
   char op_8_transB = 'n';
   int op_8_m = 1;
   int op_8_n = 3;
   int op_8_k = 32;
   float op_8_alpha = 1;
   float op_8_beta = 1;
   int op_8_lda = 32;
   int op_8_ldb = 3;
   std::copy(tensor_dense4bias0bcast, tensor_dense4bias0bcast + 3, tensor_dense4BiasAdd0);
   BLAS::sgemm_(&op_8_transB, &op_8_transA, &op_8_n, &op_8_m, &op_8_k, &op_8_alpha, tensor_dense4kernel0, &op_8_ldb, tensor_dense3Tanh0, &op_8_lda, &op_8_beta, tensor_dense4BiasAdd0, &op_8_n);
   std::vector<float> ret (tensor_dense4BiasAdd0, tensor_dense4BiasAdd0 + 3);
   return ret;
}
};
} //TMVA_SOFIE_trainedTaggerRegressionModel

#endif  // TMVA_SOFIE_TRAINEDTAGGERREGRESSIONMODEL
