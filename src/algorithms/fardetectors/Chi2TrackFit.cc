// Created by Simon Gardner to do LowQ2 tracking
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4hep/Vector2f.h>
#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4hep/TrackerHitCollection.h>

#include "Chi2TrackFit.h"
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <ROOT/RVec.hxx>
#include <TDecompSVD.h>
#include <TMatrixD.h>
#include <Math/Point3D.h>
#include <Math/Vector3D.h>


namespace eicrecon {


    void Chi2TrackFit::init() {

    }

    std::unique_ptr<edm4eic::TrackParametersCollection> Chi2TrackFit::produce(const edm4hep::TrackerHitCollection &inputhits) {
      //    std::unique_ptr<edm4eic::TrackPointCollection> Chi2TrackFit::produce(const edm4hep::TrackerHitCollection &inputhits) {

        auto outputTracks = std::make_unique<edm4eic::TrackParametersCollection>();
	//        auto outputTracks = std::make_unique<edm4eic::TrackPointCollection>();

	std::map<int,std::map<int,std::vector<edm4hep::TrackerHit>>> sortedHits;

	// Sort the hits by module and layer
	for(auto hit: inputhits){
	  auto module = m_id_dec->get(hit.getCellID(),m_cfg.detconf.module_idx);
	  auto layer  = m_id_dec->get(hit.getCellID(),m_cfg.detconf.layer_idx);
	  sortedHits[module][layer].push_back(hit);
	}

	// Loop over module
	for ( auto moduleHits : sortedHits ) {

	  // Check there is a hit in each layer of the module
	  if(moduleHits.second.size()<4) continue;

	  // For matrix vector fitting method
	  ROOT::VecOps::RVec<float> x(4,0);
	  ROOT::VecOps::RVec<float> y(4,0);
	  ROOT::VecOps::RVec<float> z(4,0);
	  TMatrixD ma(3,1);
	  TMatrixD mb(3,1);
	  TMatrixD mc(3,1);
	  TMatrixD md(3,1);

	  // For changing how strongly each layer hit is in contributing to the fit
	  double layerWeights[4] = {1,1,1,1};
	  double meanWeight = 1;
	  int    layerLimit = 5;

	  // Temporary limit of number of hits per layer before Kalman filtering/GNN implemented
	  // TODO - Implement more sensible solution
	  if( moduleHits.second[0].size()>layerLimit ) break;
	  if( moduleHits.second[1].size()>layerLimit ) break;
	  if( moduleHits.second[2].size()>layerLimit ) break;
	  if( moduleHits.second[3].size()>layerLimit ) break;

	  // Loop over hits in every layer
	  // TODO - Implement more sensible solution
	  for ( auto hit0c : moduleHits.second[0] ) {
	    auto hit0 = ROOT::Math::XYZVector(hit0c.getPosition().x,hit0c.getPosition().y,hit0c.getPosition().z);
	    x[0] = hit0.x()*layerWeights[0];
	    y[0] = hit0.y()*layerWeights[0];
	    z[0] = hit0.z()*layerWeights[0];
	    for ( auto hit1c : moduleHits.second[1] ) {
	      auto hit1 = ROOT::Math::XYZVector(hit1c.getPosition().x,hit1c.getPosition().y,hit1c.getPosition().z);
	      x[1] = hit1.x()*layerWeights[1];
	      y[1] = hit1.y()*layerWeights[1];
	      z[1] = hit1.z()*layerWeights[1];
	      for ( auto hit2c : moduleHits.second[2] ) {
		auto hit2 = ROOT::Math::XYZVector(hit2c.getPosition().x,hit2c.getPosition().y,hit2c.getPosition().z);
		x[2] = hit2.x()*layerWeights[2];
		y[2] = hit2.y()*layerWeights[2];
		z[2] = hit2.z()*layerWeights[2];
		for ( auto hit3c : moduleHits.second[3] ) {
		  auto hit3 = ROOT::Math::XYZVector(hit3c.getPosition().x,hit3c.getPosition().y,hit3c.getPosition().z);
		  x[3] = hit3.x()*layerWeights[3];
		  y[3] = hit3.y()*layerWeights[3];
		  z[3] = hit3.z()*layerWeights[3];

		  // Weighted track centre
		  ROOT::Math::XYZVector outPos = ROOT::Math::XYZVector(Mean(x),Mean(y),Mean(z))/meanWeight;

		  // Fill matrices with weighted hit points
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
		  auto subMat    = varMatrix.GetSub(0,3,1,2);

		  auto subAsArray  = subMat.GetMatrixArray();
		  ROOT::VecOps::RVec<double> subAsVector(subAsArray,subAsArray+8);
		  double outChi2 = Sum(subAsVector*subAsVector)/8;

		  // Chi2 cut on fit to cluster points
		  if(outChi2>0.001) continue; // Optimise later or add as config

		  ROOT::Math::XYZVector outVec(decompVec[0],decompVec[3],decompVec[6]);

		  // Make sure fit was pointing in the right direction
		  if(outVec.Z()>0) outVec*=-1;

		  //position vector crosses x axis, swap to z exit later
		  auto exitPos = outPos-(outPos.x()/outVec.x())*outVec;

		  // Create track parameters edm4eic structure
		  // TODO - populate more of the fields
		  int type = 0;
		  // Plane Point
		  edm4hep::Vector2f loc(exitPos.y()*10,exitPos.z()*10); //Temp unit transform
		  // Point Error
		  edm4eic::Cov2f locError;
		  float theta = outVec.Unit().Theta();
		  float phi   = outVec.Unit().Phi();
		  edm4eic::Cov2f dirError;
		  float qOverP;
		  edm4eic::Cov3f momentumError;
		  float time      = 0;
		  float timeError = 0;
		  float charge    = 0;	
		  float path      = 0;
		  float pathError = 0;
		  
 		  edm4eic::TrackParameters outTrack(type,loc,locError,theta,phi,qOverP,momentumError,time,timeError,charge);
 		  outputTracks->push_back(outTrack);

		  // Plane Point
		  edm4hep::Vector3f pos(exitPos.x()*10,exitPos.y()*10,exitPos.z()*10); //Temp unit transform
		  edm4eic::Cov3f    posError;

		  
		  edm4hep::Vector3f mom(outVec.x(),outVec.y(),outVec.z()); //Temp unit transform
		  edm4eic::Cov3f    momError;


// 		  edm4eic::TrackPoint outTrack(pos,posError,mom,momError,time,timeError,theta,phi,dirError,path,pathError);
// 		  outputTracks->push_back(outTrack);

		}
	      }
	    }
	  }


	}

	return outputTracks;

    }

}
