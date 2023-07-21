// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Alex Jentsch, Wouter Deconinck, Sylvester Joosten, David Lawrence
//
// This converted from: https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/FarForwardParticles.cpp

#include "MatrixTransferStatic.h"

void eicrecon::MatrixTransferStatic::init(std::shared_ptr<spdlog::logger> &logger) {

  m_log = logger;
  
  // do not get the layer/sector ID if no readout class provided
  if (m_readout.empty()) return;
  
  auto id_spec = detector->readout(m_readout).idSpec();
  try {
    id_dec = id_spec.decoder();
    if (!m_sectorField.empty()) {
      sector_idx = id_dec->index(m_sectorField);
      m_log->info("Find sector field {}, indes = {}", m_sectorField, sector_idx);
    }
    if (!m_layerField.empty()) {
      layer_idx = id_dec->index(m_layerField);
      m_log->info("Find layer field {}, index = {}", m_layerField, layer_idx);
    }
  } catch (...) {
    m_log->error("Failed to load ID decoder for {}", m_readout);
    return;
  }
  
  // local detector name has higher priority
  if (!m_localDetElement.empty()) {
    try {
      local = detector->detector(m_localDetElement);
      m_log->info("Local coordinate system from DetElement {}", m_localDetElement);
    } catch (...) {
      m_log->error("Failed to locate local coordinate system from DetElement {}", m_localDetElement);
      return;
    }
    // or get from fields
  } else {
    std::vector<std::pair<std::string, int>> fields;
    for (auto &f: u_localDetFields) {
      fields.emplace_back(f, 0);
    }
  }
  
  //Calculate inverse of static transfer matrix
  std::vector<std::vector<double>> aX(m_cfg.aX);
  std::vector<std::vector<double>> aY(m_cfg.aY);

  double det = aX[0][0] * aX[1][1] - aX[0][1] * aX[1][0];
  
  if (det == 0) {
    m_log->error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
    return;
  }
  
  aXinv[0][0] =  aX[1][1] / det;
  aXinv[0][1] = -aX[0][1] / det;
  aXinv[1][0] = -aX[1][0] / det;
  aXinv[1][1] =  aX[0][0] / det;


  det = aY[0][0] * aY[1][1] - aY[0][1] * aY[1][0];

  if (det == 0) {
    m_log->error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
    return;
  }
  
  aYinv[0][0] =  aY[1][1] / det;
  aYinv[0][1] = -aY[0][1] / det;
  aYinv[1][0] = -aY[1][0] / det;
  aYinv[1][1] =  aY[0][0] / det;
  
  return;

}

std::vector<edm4eic::ReconstructedParticle*> eicrecon::MatrixTransferStatic::produce(const std::vector<const edm4hep::SimTrackerHit *>& rawhits) {
       
  auto converter = m_cellid_converter;

  std::vector<edm4eic::ReconstructedParticle*> outputParticles;

  //---- begin Reconstruction code ----

        int eventReset = 0; // counter for IDing at least one hit per layer
        std::vector<double> hitx;
        std::vector<double> hity;
        std::vector<double> hitz;

        for (const auto &h: rawhits) {

            auto cellID = h->getCellID();
            // The actual hit position in Global Coordinates
            // auto pos0 = h.position();

            auto gpos = converter->position(cellID);
            // local positions
            if (m_localDetElement.empty()) {
                auto volman = detector->volumeManager();
                local = volman.lookupDetElement(cellID);
            }
            auto pos0 = local.nominal().worldToLocal(
                    dd4hep::Position(gpos.x(), gpos.y(), gpos.z())); // hit position in local coordinates

            // auto mom0 = h.momentum;
            // auto pidCode = h.g4ID;
            auto eDep = h->getEDep();

            if (eDep < 0.00001) {
                continue;
            }

            if (eventReset < 2) {
                hitx.push_back(pos0.x()); // - local_x_offset_station_2);
            }                           // use station 2 for both offsets since it is used for the reference orbit
            else {
                hitx.push_back(pos0.x()); // - local_x_offset_station_2);
            }

            hity.push_back(pos0.y());
            hitz.push_back(pos0.z());

            eventReset++;
        }

        // NB:
        // This is a "dumb" algorithm - I am just checking the basic thing works with a simple single-proton test.
        // Will need to update and modify for generic # of hits for more complicated final-states.

        if (eventReset == 4) {

            // extract hit, subtract orbit offset – this is to get the hits in the coordinate system of the orbit
            // trajectory
            double XL[2] = {hitx[0], hitx[2]};
            double YL[2] = {hity[0], hity[2]};

            double base = hitz[2] - hitz[0];

            if (base == 0) {
                m_log->info("Detector separation = 0! Cannot calculate slope!");
                return outputParticles;
            }

            double Xip[2] = {0.0, 0.0};
            double Xrp[2] = {XL[1], (1000 * (XL[1] - XL[0]) / (base)) - m_cfg.local_x_slope_offset}; //- _SX0RP_;
            double Yip[2] = {0.0, 0.0};
            double Yrp[2] = {YL[1], (1000 * (YL[1] - YL[0]) / (base)) - m_cfg.local_y_slope_offset}; //- _SY0RP_;

            // use the hit information and calculated slope at the RP + the transfer matrix inverse to calculate the
            // Polar Angle and deltaP at the IP

            for (unsigned i0 = 0; i0 < 2; i0++) {
                for (unsigned i1 = 0; i1 < 2; i1++) {
                    Xip[i0] += aXinv[i0][i1] * Xrp[i1];
                    Yip[i0] += aYinv[i0][i1] * Yrp[i1];
                }
            }

            // convert polar angles to radians
            double rsx = Xip[1] / 1000.;
            double rsy = Yip[1] / 1000.;

            // calculate momentum magnitude from measured deltaP – using thin lens optics.
            double p = m_cfg.nomMomentum * (1 + 0.01 * Xip[0]);
            double norm = std::sqrt(1.0 + rsx * rsx + rsy * rsy);

            const float prec[3] = {static_cast<float>(p * rsx / norm), static_cast<float>(p * rsy / norm),
                                   static_cast<float>(p / norm)};

            //----- end reconstruction code ------

            edm4eic::MutableReconstructedParticle reconTrack;
            reconTrack.setType(0);
            reconTrack.setMomentum({prec});
            reconTrack.setEnergy(std::hypot(edm4eic::magnitude(reconTrack.getMomentum()), m_cfg.partMass));
            reconTrack.setReferencePoint({0, 0, 0});
            reconTrack.setCharge(m_cfg.partCharge);
            reconTrack.setMass(m_cfg.partMass);
            reconTrack.setGoodnessOfPID(1.);
            reconTrack.setPDG(m_cfg.partPDG);
            //reconTrack.covMatrix(); // @TODO: Errors
            outputParticles.push_back(new edm4eic::ReconstructedParticle(reconTrack));

        } // end enough hits for matrix reco

	return outputParticles;

    }
