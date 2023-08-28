// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Alex Jentsch, Wouter Deconinck, Sylvester Joosten, David Lawrence
//
// This converted from: https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/FarForwardParticles.cpp

#include <algorithm>
#include <cmath>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/Surface.h>
#include <DDRec/SurfaceManager.h>


// Event Model related classes
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/MutableReconstructedParticle.h>
#include <edm4eic/TrackerHit.h>
#include <edm4eic/vector_utils.h>

class FarForwardParticles {
public:
    std::vector<const edm4eic::TrackerHit *> m_inputHits;
    std::vector<edm4eic::ReconstructedParticle *> m_outputParticles;

    //----- Define constants here ------

    double local_x_offset_station_1; // -833.3878326
    double local_x_offset_station_2; // -924.342804
    double local_x_slope_offset; // -0.00622147
    double local_y_slope_offset; // -0.0451035
    double crossingAngle; // -0.025
    double nomMomentum; // 275.0

    std::string m_readout;
    std::string m_layerField;
    std::string m_sectorField;

    dd4hep::BitFieldCoder *id_dec = nullptr;
    size_t sector_idx{0}, layer_idx{0};

    std::string m_localDetElement;
    std::vector<std::string> u_localDetFields;

    dd4hep::DetElement local;
    size_t local_mask = ~0;
    std::shared_ptr<spdlog::logger> m_log;
    dd4hep::Detector *detector = nullptr;
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter = nullptr;

    const double aXRP[2][2] = {{2.102403743, 29.11067626},
                               {0.186640381, 0.192604619}};
    const double aYRP[2][2] = {{0.0000159900, 3.94082098},
                               {0.0000079946, -0.1402995}};

    double aXRPinv[2][2] = {{0.0, 0.0},
                            {0.0, 0.0}};
    double aYRPinv[2][2] = {{0.0, 0.0},
                            {0.0, 0.0}};

public:
    FarForwardParticles() {}

    // See Wouter's example to extract local coordinates CalorimeterHitReco.cpp
    // includes DDRec/CellIDPositionConverter.here
    // See tutorial
    // auto converter = m_GeoSvc ....
    // https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/CalorimeterHitReco.cpp - line 200
    // include the Eigen libraries, used in ACTS, for the linear algebra.

    void initialize(std::shared_ptr<spdlog::logger> &logger) {
        m_log = logger;

        // do not get the layer/sector ID if no readout class provided
        if (m_readout.empty()) return;

        auto id_spec = detector->readout(m_readout).idSpec();
        try {
            id_dec = id_spec.decoder();
            if (!m_sectorField.empty()) {
                sector_idx = id_dec->index(m_sectorField);
                m_log->info("Find sector field {}, index = {}", m_sectorField, sector_idx);
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
            local_mask = id_spec.get_mask(fields);
            // use all fields if nothing provided
            if (fields.empty()) {
                local_mask = ~0;
            }
            // info() << fmt::format("Local DetElement mask {:#064b} from fields [{}]", local_mask,
            //                      fmt::join(fields, ", "))
            //        << endmsg;
        }

        double det = aXRP[0][0] * aXRP[1][1] - aXRP[0][1] * aXRP[1][0];

        if (det == 0) {
            m_log->error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
            return;
        }

        aXRPinv[0][0] = aXRP[1][1] / det;
        aXRPinv[0][1] = -aXRP[0][1] / det;
        aXRPinv[1][0] = -aXRP[1][0] / det;
        aXRPinv[1][1] = aXRP[0][0] / det;

        det = aYRP[0][0] * aYRP[1][1] - aYRP[0][1] * aYRP[1][0];
        aYRPinv[0][0] = aYRP[1][1] / det;
        aYRPinv[0][1] = -aYRP[0][1] / det;
        aYRPinv[1][0] = -aYRP[1][0] / det;
        aYRPinv[1][1] = aYRP[0][0] / det;

        return;
    }

    void execute() {
        auto &rawhits = m_inputHits;
//            auto &rc = *(m_outputParticles.createAndPut());
//        std::vector<edm4eic::MutableReconstructedParticle *> rc;

        auto converter = m_cellid_converter;

        // for (const auto& part : mc) {
        //    if (part.genStatus() > 1) {
        //        if (msgLevel(MSG::DEBUG)) {
        //            debug() << "ignoring particle with genStatus = " << part.genStatus() << endmsg;
        //        }
        //        continue;
        //    }

        //---- begin Roman Pot Reconstruction code ----

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
            auto eDep = h->getEdep();

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
                return;
            }

            double Xip[2] = {0.0, 0.0};
            double Xrp[2] = {XL[1], (1000 * (XL[1] - XL[0]) / (base)) - local_x_slope_offset}; //- _SX0RP_;
            double Yip[2] = {0.0, 0.0};
            double Yrp[2] = {YL[1], (1000 * (YL[1] - YL[0]) / (base)) - local_y_slope_offset}; //- _SY0RP_;

            // use the hit information and calculated slope at the RP + the transfer matrix inverse to calculate the
            // Polar Angle and deltaP at the IP

            for (unsigned i0 = 0; i0 < 2; i0++) {
                for (unsigned i1 = 0; i1 < 2; i1++) {
                    Xip[i0] += aXRPinv[i0][i1] * Xrp[i1];
                    Yip[i0] += aYRPinv[i0][i1] * Yrp[i1];
                }
            }

            // convert polar angles to radians
            double rsx = Xip[1] / 1000.;
            double rsy = Yip[1] / 1000.;

            // calculate momentum magnitude from measured deltaP – using thin lens optics.
            double p = nomMomentum * (1 + 0.01 * Xip[0]);
            double norm = std::sqrt(1.0 + rsx * rsx + rsy * rsy);

            const float prec[3] = {static_cast<float>(p * rsx / norm), static_cast<float>(p * rsy / norm),
                                   static_cast<float>(p / norm)};

            //----- end RP reconstruction code ------

            edm4eic::MutableReconstructedParticle rpTrack;
            rpTrack.setType(0);
            rpTrack.setMomentum({prec});
            rpTrack.setEnergy(std::hypot(edm4eic::magnitude(rpTrack.getMomentum()), .938272));
            rpTrack.setReferencePoint({0, 0, 0});
            rpTrack.setCharge(1);
            rpTrack.setMass(.938272);
            rpTrack.setGoodnessOfPID(1.);
            rpTrack.setPDG(2122);
            //rpTrack.covMatrix(); // @TODO: Errors
            m_outputParticles.push_back(new edm4eic::ReconstructedParticle(rpTrack));

        } // end enough hits for matrix reco

        return;
    }
};
