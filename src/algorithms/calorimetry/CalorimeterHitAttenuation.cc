// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#include "CalorimeterHitAttenuation.h"

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DD4hep/config.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <DD4hep/Alignments.h>
#include <DD4hep/Handle.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Segmentations.h>
#include <DD4hep/Shapes.h>
#include <DD4hep/VolumeManager.h>
#include <DD4hep/Volumes.h>
#include <DD4hep/detail/SegmentationsInterna.h>
#include <DDSegmentation/MultiSegmentation.h>
#include <DDSegmentation/Segmentation.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/service.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <fmt/format.h>
#include <cctype>
#include <map>
#include <ostream>

#include "algorithms/calorimetry/CalorimeterHitAttenuationConfig.h"
#include "services/evaluator/EvaluatorSvc.h"

using namespace dd4hep;

namespace eicrecon{

        void CalorimeterHitAttenuation::init(){

        }

        void CalorimeterHitAttenuation::process(const CalorimeterHitAttenuation::Input& input,
                                                const CalorimeterHitAttenuation::Output& output) const{

                const auto [in_hits] = input;
                auto [out_hits] = output;

                /*for(size_t i=0; i<in_hits->size(); i++){
                        auto simhit = (*in_hits)[i];
                        //auto simhit3 = out_hits->create();

                        edm4hep::MutableSimCalorimeterHit simhit2;
                        //simhit2.setCellID(cellID);
                        edm4hep::MutableCaloHitContribution cont;
                        edm4hep::Vector3f position {1, 1, 1};

                        out_hits->create(simhit.getCellID(),
                                         simhit.getEnergy(),
                                         position,
                                         cont);



                        //info("energy = {}", simhit.getEnergy());
                        //out_hits->push_back(simhit2);
                        //info("CalorimeterHitAttenuationTest> push_back()");
                }

                info("CalorimeterHitAttenuationTest> after for loop");*/


                std::size_t ix = 0;
                for(const auto &ih : *in_hits){
                        /*edm4hep::MutableSimCalorimeterHit simhit;
                        simhit.setCellID(ih.getCellID());
                        simhit.setEnergy(ih.getEnergy());
                        simhit.setPosition(ih.getPosition());
                        simhit.setContributions(ih.getContributions());*/

                        auto simhit = ih.clone();

                        out_hits->push_back(simhit);

                        //info("CalorimeterHitAttenuationTest> push_back()");
                }

        }

}
