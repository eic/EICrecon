
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck

#pragma once

#include <random>

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4eic/ProtoCluster.h>
#include <edm4eic/MutableProtoCluster.h>
#include <spdlog/spdlog.h>



class CalorimeterTruthClustering {

protected:
    // Insert any member variables here
    std::shared_ptr<spdlog::logger> m_log;

public:
    CalorimeterTruthClustering() = default;
    ~CalorimeterTruthClustering(){} // better to use smart pointer?
    virtual void AlgorithmInit(std::shared_ptr<spdlog::logger> &logger) ;
    virtual void AlgorithmChangeRun() ;
    virtual void AlgorithmProcess() ;

    //-------- Configuration Parameters ------------

  std::vector<const edm4eic::CalorimeterHit*> m_inputHits;//{"inputHits", Gaudi::DataHandle::Reader, this};
  std::vector<const edm4hep::SimCalorimeterHit*> m_mcHits;//{"mcHits", Gaudi::DataHandle::Reader, this};

  std::vector<edm4eic::ProtoCluster*> m_outputProtoClusters;//{"outputProtoClusters", Gaudi::DataHandle::Writer, this};

private:

};
