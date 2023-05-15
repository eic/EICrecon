// Created by Simon Gardner to do LowQ2 reconstruction
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4hep/Vector2f.h>
#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/TrackParametersCollection.h>

#include "LowQ2Tracking_factory.h"
#include "LowQ2Cluster_factory.h"
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/string/StringHelpers.h>
#include <ROOT/RVec.hxx>
#include <TDecompSVD.h>
#include <TMatrixD.h>


namespace eicrecon {


    void LowQ2Tracking_factory::Init() {

	auto app = GetApplication();

	m_log = app->GetService<Log_service>()->logger(m_output_tag);

	m_log->info("LowQ2 Tracking Built...");

    }


    void LowQ2Tracking_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    	// Nothing to do here
    }

    void LowQ2Tracking_factory::Process(const std::shared_ptr<const JEvent> &event) {


	std::vector<edm4eic::TrackParameters*> outputTracks;

	auto inputhits = event->Get<eicrecon::TrackerClusterPoint>(m_input_tag);

	std::map<int,std::map<int,std::vector<eicrecon::TrackerClusterPoint>>> sortedHits;

	for(auto hit: inputhits){
	  auto module = hit->pCluster->module;
	  auto layer  = hit->pCluster->layer;
	  sortedHits[module][layer].push_back(*hit);
	}
	
	for ( auto moduleHits : sortedHits ) {
	  //std::cout << "mod " << moduleHits.first << " " << moduleHits.second.size() << "\n";
	  if(moduleHits.second.size()<4) continue;
	  
	  ROOT::VecOps::RVec<float> x(4,0);
	  ROOT::VecOps::RVec<float> y(4,0);
	  ROOT::VecOps::RVec<float> z(4,0);
	  TMatrixD ma(3,1);
	  TMatrixD mb(3,1);
	  TMatrixD mc(3,1);
	  TMatrixD md(3,1);
	  double layerWeights[4] = {1,1,1,1};
	  double meanWeight = 1;

	  //temporary limits before Kalman filtering implemented
	  if( moduleHits.second[0].size()>2 ) break;
	  if( moduleHits.second[1].size()>2 ) break;
	  if( moduleHits.second[2].size()>2 ) break;
	  if( moduleHits.second[3].size()>2 ) break;

	  for ( auto hit0c : moduleHits.second[0] ) {
	    auto hit0 = ROOT::Math::XYZVector(hit0c.position.x,hit0c.position.y,hit0c.position.z);
	    x[0] = hit0.x()*layerWeights[0];
	    y[0] = hit0.y()*layerWeights[0];
	    z[0] = hit0.z()*layerWeights[0];
	    for ( auto hit1c : moduleHits.second[1] ) {
	      auto hit1 = ROOT::Math::XYZVector(hit1c.position.x,hit1c.position.y,hit1c.position.z);
	      x[1] = hit1.x()*layerWeights[1];
	      y[1] = hit1.y()*layerWeights[1];
	      z[1] = hit1.z()*layerWeights[1];
	      for ( auto hit2c : moduleHits.second[2] ) {
		auto hit2 = ROOT::Math::XYZVector(hit2c.position.x,hit2c.position.y,hit2c.position.z);
		x[2] = hit2.x()*layerWeights[2];
		y[2] = hit2.y()*layerWeights[2];
		z[2] = hit2.z()*layerWeights[2];
		for ( auto hit3c : moduleHits.second[3] ) {
		  auto hit3 = ROOT::Math::XYZVector(hit3c.position.x,hit3c.position.y,hit3c.position.z);
		  x[3] = hit3.x()*layerWeights[3];
		  y[3] = hit3.y()*layerWeights[3];
		  z[3] = hit3.z()*layerWeights[3];

		  ROOT::Math::XYZVector outPos = ROOT::Math::XYZVector(Mean(x),Mean(y),Mean(z))/meanWeight;


		  ROOT::Math::XYZPoint((hit0-outPos)*layerWeights[0]).GetCoordinates(ma.GetMatrixArray());
		  ROOT::Math::XYZPoint((hit1-outPos)*layerWeights[1]).GetCoordinates(mb.GetMatrixArray());
		  ROOT::Math::XYZPoint((hit2-outPos)*layerWeights[2]).GetCoordinates(mc.GetMatrixArray());
		  ROOT::Math::XYZPoint((hit3-outPos)*layerWeights[3]).GetCoordinates(md.GetMatrixArray());
		  
		  
		  TMatrixD vecMatrix(3,4);
		  vecMatrix.SetSub(0,0,ma);
		  vecMatrix.SetSub(0,1,mb);
		  vecMatrix.SetSub(0,2,mc);
		  vecMatrix.SetSub(0,3,md);
		  
		  TDecompSVD decomp(vecMatrix.T());
		  
		  decomp.Decompose();		  
		  
		  auto decompVec = decomp.GetV().GetMatrixArray();
		  
		  auto varMatrix = vecMatrix*(decomp.GetV());
		  //varMatrix.Print();
		  auto subMat = varMatrix.GetSub(0,3,1,2);
		  // 	      subMat.Print();
		  auto subAsArray  = subMat.GetMatrixArray();
		  ROOT::VecOps::RVec<double> subAsVector(subAsArray,subAsArray+8);
		  double outChi2 = Sum(subAsVector*subAsVector)/8;

		  if(outChi2>0.0001) continue; // Optimise later or add as config
		  
		  // 	      lf->AssignData(maxLayer, 2, &v[0], &z[0]);  
		  // 	      lf->Eval();
		  TVectorD params;
		  TVectorD errors;
		  // 	      lf->GetParameters(params);
		  // 	      params.Print();
		  // 	      lf->GetErrors(errors);
		  //double outChi2=1;
		  // 	      double outChi2=lf->GetChisquare();
		  //	      XYZVector outVec = XYZVector(params[0],params[1],params[0]);
		  ROOT::Math::XYZVector outVec(decompVec[0],decompVec[3],decompVec[6]);
		  // 	      std::cout << outPos << std::endl;
		  if(outVec.Z()>0) outVec*=-1;
		  // 	      std::cout << outChi2 << std::endl<< std::endl;	      
		  //track outTrack = {outPos,outVec.Unit(),outChi2,index};
		  
		  //Position vector crosses x axis, swap to z exit later
		  auto exitPos = outPos-(outPos.x()/outVec.x())*outVec;
		  //auto exitPos = 

		  int type = 0;
		  // Plane Point
		  edm4hep::Vector2f loc(exitPos.y()*10,exitPos.z()*10); //Temp unit transform
		  // Point Error
		  edm4eic::Cov2f locError;
		  float theta = outVec.Unit().Theta();
		  float phi   = outVec.Unit().Phi();
		  float qOverP;
		  edm4eic::Cov3f momentumError;
		  float time  = 0;
		  float timeError = 0;
		  float charge = 0;
 
		  edm4eic::TrackParameters* outTrack =
		    new edm4eic::TrackParameters(type,loc,locError,theta,phi,qOverP,momentumError,time,timeError,charge);
		  outputTracks.push_back(outTrack);
		  
		}
	      }
	    }
	  }
	  

	}

// 	outputTracks.push_back(new edm4eic::TrackParameters(track));

        
	Set(outputTracks);

    }

}
