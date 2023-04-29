
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Wouter Deconinck, Chao, Whitney Armstrong

// Reconstruct digitized outputs, paired with Jug::Digi::CalorimeterHitDigi
// Author: Chao Peng
// Date: 06/14/2021

#pragma once

#include <random>

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/RawCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/vector_utils.h>
#include <spdlog/spdlog.h>


class CalorimeterHitReco {

    // Insert any member variables here

public:
    CalorimeterHitReco() = default;
    ~CalorimeterHitReco(){} // better to use smart pointer?
    virtual void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
    virtual void AlgorithmChangeRun() ;
    virtual void AlgorithmProcess() ;

    //-------- Configuration Parameters ------------
    //instantiate new spdlog logger
    std::shared_ptr<spdlog::logger> m_log;
    // Name of input data type (collection)
    std::string              m_input_tag;

    double m_lUnit = 1.0 * dd4hep::mm;

  // digitization settings, must be consistent with digi class
  unsigned int m_capADC;//{this, "capacityADC", 8096};
  double m_dyRangeADC;//{this, "dynamicRangeADC", 100. * dd4hep::MeV};
  unsigned int m_pedMeanADC;//{this, "pedestalMean", 400};
  double m_pedSigmaADC;//{this, "pedestalSigma", 3.2};
  double m_resolutionTDC;//{this, "resolutionTDC", 10 * ps};

  // zero suppression values
  double m_thresholdFactor;//{this, "thresholdFactor", 0.0};
  double m_thresholdValue;//{this, "thresholdValue", 0.0};

  // energy correction with sampling fraction
  double m_sampFrac          = 1.;//{this, "samplingFraction", 1.0};
  double m_sampFracLayer[14] = {0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                                0., 0., 0., 0.}; //maximum 14 layers

  // unitless counterparts of the input parameters
  double thresholdADC{0};
  double stepTDC{0};

    std::shared_ptr<JDD4hep_service> m_geoSvc;
  //DataHandle<edm4eic::RawCalorimeterHitCollection> m_inputHitCollection{"inputHitCollection", Gaudi::DataHandle::Reader,  this};
  //DataHandle<edm4eic::CalorimeterHitCollection> m_outputHitCollection{"outputHitCollection", Gaudi::DataHandle::Writer,   this};

  // geometry service to get ids, ignored if no names provided
  std::string m_geoSvcName="geoServiceName";
  std::string m_readout="readoutClass";
  std::string m_layerField="layerField";
  std::string m_sectorField="sectorField";

  dd4hep::BitFieldCoder* id_dec = nullptr;
  uint32_t NcellIDerrors = 0;
  uint32_t MaxCellIDerrors = 100;

  size_t sector_idx{0}, layer_idx{0};

  // name of detelment or fields to find the local detector (for global->local transform)
  // if nothing is provided, the lowest level DetElement (from cellID) will be used
  std::string m_localDetElement="", m_maskPos="";
  std::vector<std::string> u_localDetFields={}, u_maskPosFields={};
  dd4hep::DetElement local;
  size_t local_mask = ~0, gpos_mask = 0;

    std::vector<edm4eic::CalorimeterHit*> hits;
    std::vector<const edm4hep::RawCalorimeterHit*> rawhits;

private:
    //std::default_random_engine generator; // TODO: need something more appropriate here
    //std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1
};
