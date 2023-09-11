// Created by Alex Jentsch to do Roman Pots reconstruction
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include "OffMomentumReconstruction_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"

namespace eicrecon {


    OffMomentumReconstruction_factory::OffMomentumReconstruction_factory(){ SetTag(m_output_tag); }

    void OffMomentumReconstruction_factory::Init() {

        std::string plugin_name = GetPluginName();
        std::string param_prefix = plugin_name + ":" + m_input_tag + ":";

        auto *app = GetApplication();

        m_log = app->GetService<Log_service>()->logger(m_output_tag);

        m_readout = m_input_tag;

        app->SetDefaultParameter(param_prefix+"readoutClass", m_readout);
        m_geoSvc = app->GetService<JDD4hep_service>();

        if(m_readout.empty()){ m_log->error("READOUT IS EMPTY!"); return; }


        double det = aXOMD[0][0] * aXOMD[1][1] - aXOMD[0][1] * aXOMD[1][0];

        if (det == 0) {
            m_log->error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
            throw JException("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
        }

        aXOMDinv[0][0] = aXOMD[1][1] / det;
        aXOMDinv[0][1] = -aXOMD[0][1] / det;
        aXOMDinv[1][0] = -aXOMD[1][0] / det;
        aXOMDinv[1][1] = aXOMD[0][0] / det;

        det = aYOMD[0][0] * aYOMD[1][1] - aYOMD[0][1] * aYOMD[1][0];

        if (det == 0) {
            m_log->error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
            throw JException("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
        }

        aYOMDinv[0][0] = aYOMD[1][1] / det;
        aYOMDinv[0][1] = -aYOMD[0][1] / det;
        aYOMDinv[1][0] = -aYOMD[1][0] / det;
        aYOMDinv[1][1] = aYOMD[0][0] / det;


    }


    void OffMomentumReconstruction_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void OffMomentumReconstruction_factory::Process(const std::shared_ptr<const JEvent> &event) {


        std::vector<edm4eic::ReconstructedParticle*> outputOMDTracks;

        auto converter = m_geoSvc->cellIDPositionConverter();

        auto rawhits =  event->Get<edm4hep::SimTrackerHit>("ForwardOffMTrackerHits");


        //---- begin Roman Pot Reconstruction code ----

        int eventReset = 0; // counter for IDing at least one hit per layer
        std::vector<double> hitx;
        std::vector<double> hity;
        std::vector<double> hitz;

        double goodHitX[2] = {0.0, 0.0};
        double goodHitY[2] = {0.0, 0.0};
        double goodHitZ[2] = {0.0, 0.0};

        bool goodHit1 = false;
        bool goodHit2 = false;

        for (const auto *const h: rawhits) {

            auto cellID = h->getCellID();

            //global --> local begins here -----

            auto gpos = converter->position(cellID);

            // local positions
            //if (m_localDetElement.empty()) {
            auto volman = m_geoSvc->detector()->volumeManager();
            local = volman.lookupDetElement(cellID);
            //}

            auto pos0 = local.nominal().worldToLocal(dd4hep::Position(gpos.x(), gpos.y(), gpos.z())); // hit position in local coordinates

            //information is stored in cm, we need mm - divide by dd4hep::mm

            if(!goodHit2 && gpos.z()/dd4hep::mm > 24499.0 && gpos.z()/dd4hep::mm < 24522.0){

                goodHitX[1] = pos0.x()/dd4hep::mm;
                goodHitY[1] = pos0.y()/dd4hep::mm;
                goodHitZ[1] = gpos.z()/dd4hep::mm;
                goodHit2 = true;

            }
            if(!goodHit1 && gpos.z()/dd4hep::mm > 22499.0 && gpos.z()/dd4hep::mm < 22522.0){

                goodHitX[0] = pos0.x()/dd4hep::mm;
                goodHitY[0] = pos0.y()/dd4hep::mm;
                goodHitZ[0] = gpos.z()/dd4hep::mm;
                goodHit1 = true;

            }

        }// end loop over hits

        // NB:
        // This is a "dumb" algorithm - I am just checking the basic thing works with a simple single-proton test.
        // Will need to update and modify for generic # of hits for more complicated final-states.

        if (goodHit1 && goodHit2) {

            // extract hit, subtract orbit offset – this is to get the hits in the coordinate system of the orbit
            // trajectory -- should eventually be in local coordinates.

            double XL[2] = {goodHitX[0] - local_x_offset, goodHitX[1] - local_x_offset};
            double YL[2] = {goodHitY[0] - local_y_offset, goodHitY[1] - local_y_offset};

            double base = goodHitZ[1] - goodHitZ[0];



            if (base == 0) {
                m_log->info("Detector separation = 0! Cannot calculate slope!");
                throw JException("Detector separation = 0! Cannot calculate slope!");
            }

            double Xip[2] = {0.0, 0.0};
            double Xrp[2] = {XL[1], (1000 * (XL[1] - XL[0]) / (base)) - local_x_slope_offset}; //- _SX0RP_;
            double Yip[2] = {0.0, 0.0};
            double Yrp[2] = {YL[1], (1000 * (YL[1] - YL[0]) / (base)) - local_y_slope_offset}; //- _SY0RP_;

            // use the hit information and calculated slope at the RP + the transfer matrix inverse to calculate the
            // Polar Angle and deltaP at the IP

            for (unsigned i0 = 0; i0 < 2; i0++) {
                for (unsigned i1 = 0; i1 < 2; i1++) {
                    Xip[i0] += aXOMDinv[i0][i1] * Xrp[i1];
                    Yip[i0] += aYOMDinv[i0][i1] * Yrp[i1];
                }
            }

            // convert polar angles to radians
            double rsx = Xip[1] / 1000.;
            double rsy = Yip[1] / 1000.;

            // calculate momentum magnitude from measured deltaP – using thin lens optics.
            double p = 137.5 * (1 + 0.01 * Xip[0]);
            double norm = std::sqrt(1.0 + rsx * rsx + rsy * rsy);

            float prec[3] = {static_cast<float>((p * rsx) / norm), static_cast<float>((p * rsy) / norm),
                                   static_cast<float>(p / norm)};

            float refPoint[3] = {static_cast<float>(goodHitX[1]), static_cast<float>(goodHitY[1]), static_cast<float>(goodHitZ[1])};

            //----- end RP reconstruction code ------


            edm4eic::MutableReconstructedParticle omdTrack;
            omdTrack.setType(0);
            omdTrack.setMomentum({prec});
            omdTrack.setEnergy(std::hypot(edm4eic::magnitude(omdTrack.getMomentum()), .938272));
            omdTrack.setReferencePoint({refPoint});
            omdTrack.setCharge(1);
            omdTrack.setMass(.938272);
            omdTrack.setGoodnessOfPID(1.);
            omdTrack.setPDG(2212);
            //rpTrack.covMatrix(); // @TODO: Errors
            outputOMDTracks.push_back(new edm4eic::ReconstructedParticle(omdTrack));


        } // END matrix reco

        Set(outputOMDTracks);

    }

}
