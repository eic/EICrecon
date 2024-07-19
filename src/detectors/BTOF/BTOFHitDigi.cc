// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder

// A general digitization for BToFHit from simulation
// 1. Smear energy deposit with a/sqrt(E/GeV) + b + c/E or a/sqrt(E/GeV) (relative value)
// 2. Digitize the energy with dynamic ADC range and add pedestal (mean +- sigma)
// 3. Time conversion with smearing resolution (absolute value)
// 4. Signal is summed if the SumFields are provided
//
// Author: Souvik Paul, Chun Yuen Tsang
// Date: 19/07/2024


#include <bitset>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>
#include <vector>
#include <TGraph.h>
#include "TMath.h"
#include "TF1.h"
#include <iostream>
#include "DDRec/Surface.h"
#include "DD4hep/Detector.h"

#include "BTOFHitDigi.h"
#include "algorithms/digi/BTOFHitDigiConfig.h"

//using namespace dd4hep;
//using namespace dd4hep::Geometry;
//using namespace dd4hep::DDRec;
//using namespace eicrecon;
using namespace dd4hep::xml;

namespace eicrecon {


std::vector<bool> BTOFHitDigi::ToDigitalCode(int value, int numBits) {
    std::bitset<32> binary(value); // Convert integer to binary representation
    std::vector<bool> digitalCode;

    for (int i = numBits - 1; i >= 0; --i) {
        digitalCode.push_back(binary.test(i));
    }

    return digitalCode;
}


void BTOFHitDigi::init(const dd4hep::Detector *detector, std::shared_ptr<spdlog::logger>& logger) {
    //m_detector = detector;
    m_log = logger;
    
    adc_range = pow(2,adc_bit);
    tdc_range = pow(2,tdc_bit);
    
    // using juggler internal units (GeV, mm, radian, ns)
    tRes       = m_cfg.tRes / dd4hep::ns;
    stepTDC    = dd4hep::ns / m_cfg.resolutionTDC;

    _neighborFinder.init(detector);

}


std::unique_ptr<edm4eic::RawTrackerHitCollection> BTOFHitDigi::execute(const edm4hep::SimTrackerHitCollection *simhits) {
    //auto rawhits = std::make_unique<edm4eic::RawTrackerHitCollection>();
    auto rawhits = std::make_unique<edm4eic::RawTrackerHitCollection>();
    //const auto [sim_hits] = simhits;
    
    // find the hits that belong to the same group (for merging)
    std::unordered_map<uint64_t, std::vector<std::size_t>> merge_map;
    std::size_t ix = 0;
    for (const auto &ahit : *simhits) {
        uint64_t hid = ahit.getCellID();
        merge_map[hid].push_back(ix);

        ix++;
    }

        double thres[int(adc_range)];
        thres[0]=0.0;
        thres[1]=-0.005;
        double Vm=-0.05;
        
        for (int t = 2; t < adc_range; t++)
        {
            thres[t] = thres[1] + t*(Vm-thres[1])/(adc_range-1);
        }
        
        double scalingFactor;
        
    // signal sum
    // NOTE: we take the cellID of the most energetic hit in this group so it is a real cellID from an MC hit
    for (const auto &[id, ixs] : merge_map) {
        double edep     = 0;
        double time     = 0;//std::numeric_limits<double>::max();
        double max_edep = 0;
        auto   mid      = (*simhits)[ixs[0]].getCellID();
	if(mid == 0) continue;
        double sum_charge = 0.0;
        double sigma_sharing = 0.8;
        
        //CellIDPositionConverter converter(detector, subdet.readout());
        //Position position = converter.position(cellID);
        
        double mpv_analog = 0.0; //SP
        
        // sum energy, take time from the most energetic hit
        for (size_t i = 0; i < ixs.size(); ++i) {
            auto hit = (*simhits)[ixs[i]];

            time = hit.getTime();
            edep = hit.getEDep();
            sum_charge = edep*gain;

            // Use DetPosProcessor to process hits
            //m_detPosProcessor->ProcessSequential(hit);

            auto localPos_hit = _neighborFinder.cell2LocalPosition(mid);
            auto neighbours = _neighborFinder.findAllNeighborInSensor(mid); // Accessing NeighbourFinder through DetPosProcessor

            for (const auto& neighbour : *neighbours) {

                auto localPos_neighbour = _neighborFinder.cell2LocalPosition(neighbour);
                
                double distanceX = localPos_hit.x() - localPos_neighbour.x();
                double distanceY = localPos_hit.y() - localPos_neighbour.y();

                double exponent = -0.5 * ((pow((distanceX) / sigma_sharing, 2)) + (pow((distanceY) / sigma_sharing, 2)));
                double charge = exp(exponent) / (2 * TMath::Pi() * sigma_sharing * sigma_sharing);
                

                //std::cout<<localPos.x()<<";"<<localPos.y()<<std::endl;
                //std::cout << neighbour << " ";

            //Added by SP
//-------------------------------------------------------------
                mpv_analog = time + risetime;
                fLandau.SetParameters(mpv_analog, sigma_analog);                

                TGraph glandau;
                scalingFactor=charge/fLandau.Integral(tMin, tMax);
                for (int j = 0; j < nBins; ++j) {
                    double x = fLandau.GetXmin() + j * (fLandau.GetXmax() - fLandau.GetXmin()) / (nBins - 1);
                    double y = -1 * fLandau.Eval(x) * scalingFactor;
                    glandau.SetPoint(j, x, y);
                }

        //Added by SP
//-------------------------------------------------------------
                double intersectionX=0.0;
		int tdc = 0;
		int adc = 0;
                
                for (int j = 0; j < nBins - 1; j++) {
                    double x1, y1, x2, y2;
                    glandau.GetPoint(j, x1, y1);
                    glandau.GetPoint(j + 1, x2, y2);
                    
                    if ((y1 >= thres[1] && y2 <= thres[1])) {
                        intersectionX = x1 + (x2 - x1) * (thres[1] - y1) / (y2 - y1);
                        
                        tdc = /*BTOFHitDigi::ToDigitalCode(*/ceil(intersectionX/0.02);//, tdc_bit);
                        break;
                    }
                }

                double V=0.0;
                 
                for (int j = 0; j < nBins - 1; j++) {
                    double x1, y1, x2, y2;
                    glandau.GetPoint(j, x1, y1);
                    glandau.GetPoint(j+1, x2, y2);
                    
                    if (abs(y2) < abs(y1))//To get peak of the Analog signal
                    {
                        V=y1;
                        break;
                    }
                }
                
                adc = round(V/Vm*adc_range);
                rawhits -> create(neighbour, adc, tdc);

            }
//-----------------------------------------------------------

    }

    }
    return std::move(rawhits);

} // BTOFHitDigi:process
} // namespace eicrecon


