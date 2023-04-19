// Copyright 2023, Friederike Bock
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//  Sections Copyright (C) 2023 Friederike Bock
//  under SPDX-License-Identifier: LGPL-3.0-or-later

struct towersStrct{
  towersStrct(): energy(0), time (0), posx(0), posy(0), posz(0),  cellID(0), cellIDx(-1), cellIDy(-1), cellIDz(-1), tower_trueID(-10000), tower_clusterIDA(-1), tower_clusterIDB(-1) {}
  float energy;
  float time;
  float posx;
  float posy;
  float posz;
  int cellID;
  int cellIDx;
  int cellIDy;
  int cellIDz;
  int tower_trueID;
  int tower_clusterIDA;
  int tower_clusterIDB;
} ;

bool acompare(towersStrct lhs, towersStrct rhs) { return lhs.energy > rhs.energy; }

struct clustersStrct{
  clustersStrct(): cluster_E(0.), cluster_seed(0.), cluster_Eta(-10.), cluster_Phi(-10.), cluster_X(0.) , cluster_Y(0.), cluster_Z(0.), cluster_M02(0.), cluster_M20(0.), cluster_NTowers(0), cluster_trueID(-10000), cluster_NtrueID(0) {}
  float cluster_E;
  float cluster_seed;
  float cluster_Eta;
  float cluster_Phi;
  float cluster_X;
  float cluster_Y;
  float cluster_Z;
  float cluster_M02;
  float cluster_M20;
  int cluster_NTowers;
  int cluster_trueID;
  int cluster_NtrueID;
  std::vector<towersStrct> cluster_towers;
} ;

bool acompareCl(clustersStrct lhs, clustersStrct rhs) { return lhs.cluster_E > rhs.cluster_E; }

//**************************************************************************************************************
//**************************************************************************************************************
// find clusters with common edges or corners, separate if energy increases in neighboring cell
//**************************************************************************************************************
//**************************************************************************************************************
clustersStrct findMACluster(
                              float seed,                                     // minimum seed energy
                              float agg,                                      // minimum aggregation energy
                              std::vector<towersStrct> &input_towers_temp,    // temporary full tower array
                              std::vector<towersStrct> &cluster_towers_temp,  // towers associated to cluster
//                               std::vector<int> clslabels_temp              // MC labels in cluster
                              float aggMargin = 1.0                           // aggregation margin
                            ){
  clustersStrct tempstructC;
  if(input_towers_temp.at(0).energy > seed){
//     std::cout << "new cluster" << std::endl;
    // fill seed cell information into current cluster
    tempstructC.cluster_E       = input_towers_temp.at(0).energy;
    tempstructC.cluster_seed    = input_towers_temp.at(0).energy;
    tempstructC.cluster_NTowers = 1;
    tempstructC.cluster_NtrueID = 1;
    tempstructC.cluster_trueID = input_towers_temp.at(0).tower_trueID; // TODO save all MC labels?
    cluster_towers_temp.push_back(input_towers_temp.at(0));
//     clslabels_temp.push_back(input_towers_temp.at(0).tower_trueID);
//     std::cout  << "seed: "<<  input_towers_temp.at(0).cellIDx << "\t" << input_towers_temp.at(0).cellIDy
//                   << "\t" << input_towers_temp.at(0).cellIDz << "\t E:"<< tempstructC.cluster_E << std::endl;


    // remove seed tower from sample
    input_towers_temp.erase(input_towers_temp.begin());
    for (int tit = 0; tit < (int)cluster_towers_temp.size(); tit++){
      // Now go recursively to all neighbours and add them to the cluster if they fulfill the conditions
      int iEtaTwr = cluster_towers_temp.at(tit).cellIDx;
      int iPhiTwr = cluster_towers_temp.at(tit).cellIDy;
      int iLTwr   = cluster_towers_temp.at(tit).cellIDz;
      int refC = 0;
      for (int ait = 0; ait < (int)input_towers_temp.size(); ait++){
        int iEtaTwrAgg = input_towers_temp.at(ait).cellIDx;
        int iPhiTwrAgg = input_towers_temp.at(ait).cellIDy;
        int iLTwrAgg   = input_towers_temp.at(ait).cellIDz;

        int deltaL    = TMath::Abs(iLTwrAgg-iLTwr) ;
        int deltaPhi  = TMath::Abs(iPhiTwrAgg-iPhiTwr) ;
        int deltaEta  = TMath::Abs(iEtaTwrAgg-iEtaTwr) ;
        bool neighbor = (deltaL+deltaPhi+deltaEta == 1);
        bool corner2D = (deltaL == 0 && deltaPhi == 1 && deltaEta == 1) || (deltaL == 1 && deltaPhi == 0 && deltaEta == 1) || (deltaL == 1 && deltaPhi == 1 && deltaEta == 0);
//         first condition asks for V3-like neighbors, while second condition also checks diagonally attached towers
        if(neighbor || corner2D ){

          // only aggregate towers with lower energy than current tower

          if(input_towers_temp.at(ait).energy >= (cluster_towers_temp.at(tit).energy + aggMargin)) continue;
          tempstructC.cluster_E+=input_towers_temp.at(ait).energy;
          tempstructC.cluster_NTowers++;
          cluster_towers_temp.push_back(input_towers_temp.at(ait));
//           if(!(std::find(clslabels_temp.begin(), clslabels_temp.end(), input_towers_temp.at(ait).tower_trueID) != clslabels_temp.end())){
//             tempstructC.cluster_NtrueID++;
//             clslabels_temp.push_back(input_towers_temp.at(ait).tower_trueID);
//           }
//           std::cout << "aggregated: "<< iEtaTwrAgg << "\t" << iPhiTwrAgg << "\t" << iLTwrAgg << "\t E:" << input_towers_temp.at(ait).energy << "\t reference: "<< refC << "\t"<< iEtaTwr << "\t" << iPhiTwr << "\t" << iLTwr << "\t cond.: \t"<< neighbor << "\t" << corner2D << "\t  diffs: " << deltaEta << "\t" << deltaPhi << "\t" << deltaL<< std::endl;

          input_towers_temp.erase(input_towers_temp.begin()+ait);
          ait--;
          refC++;
        }
      }
    }
  }
  return tempstructC;
}


// ANCHOR function to determine shower shape
float * CalculateM02andWeightedPosition(std::vector<towersStrct> cluster_towers, float cluster_E_calc, float weight0){
    static float returnVariables[8]; //0:M02, 1:M20, 2:eta, 3: phi
    float w_tot = 0;
    std::vector<float> w_i;
    TVector3 vecTwr;
    TVector3 vecTwrTmp;
    float zHC     = 1;
    float w_0     = weight0;

    vecTwr = {0.,0.,0.};
    //calculation of weights and weighted position vector
    int Nweighted = 0;
    for(int cellI=0; cellI<(int)cluster_towers.size(); cellI++){
        w_i.push_back(TMath::Max( (float)0, (float) (w_0 + TMath::Log(cluster_towers.at(cellI).energy/cluster_E_calc) )));
        w_tot += w_i.at(cellI);
        if(w_i.at(cellI)>0){
          Nweighted++;
          vecTwrTmp = TVector3(cluster_towers.at(cellI).posx, cluster_towers.at(cellI).posy, cluster_towers.at(cellI).posz );
          vecTwr += w_i.at(cellI)*vecTwrTmp;
        }
    }
    // correct Eta position for average shift in calo
    returnVariables[2]= vecTwr.Eta();
    returnVariables[3]= vecTwr.Phi(); //(vecTwr.Phi()<0 ? vecTwr.Phi()+TMath::Pi() : vecTwr.Phi()-TMath::Pi());
    vecTwr*=1./w_tot;
//     std::cout << "Cluster: X: "<< vecTwr.X() << "\t" << " Y: "<< vecTwr.Y() << "\t" << " Z: "<< vecTwr.Z() << std::endl;
    returnVariables[4]=vecTwr.X();
    returnVariables[5]=vecTwr.Y();
    returnVariables[6]=vecTwr.Z();

    //calculation of M02
    float delta_phi_phi[4] = {0};
    float delta_eta_eta[4] = {0};
    float delta_eta_phi[4] = {0};
    float dispersion = 0;

    for(int cellI=0; cellI<(int)cluster_towers.size(); cellI++){
      int iphi=cluster_towers.at(cellI).cellIDy;
      int ieta=cluster_towers.at(cellI).cellIDx;
      delta_phi_phi[1] += (w_i.at(cellI)*iphi*iphi)/w_tot;
      delta_phi_phi[2] += (w_i.at(cellI)*iphi)/w_tot;
      delta_phi_phi[3] += (w_i.at(cellI)*iphi)/w_tot;

      delta_eta_eta[1] += (w_i.at(cellI)*ieta*ieta)/w_tot;
      delta_eta_eta[2] += (w_i.at(cellI)*ieta)/w_tot;
      delta_eta_eta[3] += (w_i.at(cellI)*ieta)/w_tot;

      delta_eta_phi[1] += (w_i.at(cellI)*ieta*iphi)/w_tot;
      delta_eta_phi[2] += (w_i.at(cellI)*iphi)/w_tot;
      delta_eta_phi[3] += (w_i.at(cellI)*ieta)/w_tot;

      vecTwrTmp = TVector3(cluster_towers.at(cellI).posx, cluster_towers.at(cellI).posy, cluster_towers.at(cellI).posz );
      // scale cluster position to z-plane
      vecTwr*=abs(vecTwrTmp.Z()/vecTwr.Z());
      float dx2 = pow(vecTwrTmp.X()-vecTwr.X(),2);
      float dy2 = pow(vecTwrTmp.Y()-vecTwr.Y(),2);
      float dz2 = pow(vecTwrTmp.Z()-vecTwr.Z(),2);
      dispersion+= (w_i.at(cellI)*(dx2+dy2+dz2))/w_tot;
    }
    returnVariables[7]=dispersion;
    delta_phi_phi[0] = delta_phi_phi[1] - (delta_phi_phi[2] * delta_phi_phi[3]);
    delta_eta_eta[0] = delta_eta_eta[1] - (delta_eta_eta[2] * delta_eta_eta[3]);
    delta_eta_phi[0] = delta_eta_phi[1] - (delta_eta_phi[2] * delta_eta_phi[3]);

    float calcM02 = 0.5 * ( delta_phi_phi[0] + delta_eta_eta[0] ) + TMath::Sqrt( 0.25 * TMath::Power( ( delta_phi_phi[0] - delta_eta_eta[0] ), 2 ) + TMath::Power( delta_eta_phi[0], 2 ) );
    float calcM20 = 0.5 * ( delta_phi_phi[0] + delta_eta_eta[0] ) - TMath::Sqrt( 0.25 * TMath::Power( ( delta_phi_phi[0] - delta_eta_eta[0] ), 2 ) + TMath::Power( delta_eta_phi[0], 2 ) );
//     std::cout << "M02_calc: " << calcM02 << "\t\t = 0.5 * ( " << delta_phi_phi[0] <<" + "<<delta_eta_eta[0]<<" ) + TMath::Sqrt( 0.25 * TMath::Power( ( "<<delta_phi_phi[0]<<" - "<<delta_eta_eta[0]<<" ), 2 ) + TMath::Power( "<<delta_eta_phi[0]<<", 2 ) ) "<< std::endl;
    returnVariables[0]=calcM02;
    returnVariables[1]=calcM20;
    return returnVariables;
}
