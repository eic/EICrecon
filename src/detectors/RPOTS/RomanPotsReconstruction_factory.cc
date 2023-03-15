// Created by Alex Jentsch to do Roman Pots reconstruction
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include "RomanPotsReconstruction_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/string/StringHelpers.h"

namespace eicrecon {


    RomanPotsReconstruction_factory::RomanPotsReconstruction_factory(){ SetTag("ForwardRomanPotRecParticles"); }

    void RomanPotsReconstruction_factory::Init() {

        auto app = GetApplication();

	m_log = app->GetService<Log_service>()->logger("ForwardRomanPotRecParticles");

	m_readout = "ForwardRomanPotHits";
	app->SetDefaultParameter("RPOTS:ForwardRomanPotHits:readoutClass", m_readout);
	m_geoSvc = app->GetService<JDD4hep_service>();


	if(m_readout.empty()){ std::cout << "READOUT IS EMPTY!" << std::endl;  return; }

        auto id_spec = m_geoSvc->detector()->readout(m_readout).idSpec();
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
            throw JException("Failed to load ID decoder");
        }

	std::cout << "Decoding complete..." << std::endl;

        // local detector name has higher priority
        if (!m_localDetElement.empty()) {
            try {
                local = detector->detector(m_localDetElement);
                m_log->info("Local coordinate system from DetElement {}", m_localDetElement);
            } catch (...) {
                m_log->error("Failed to locate local coordinate system from DetElement {}", m_localDetElement);
                throw JException("Failed to locate local coordinate system");
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
            throw JException("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
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


    }


    void RomanPotsReconstruction_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    	// Nothing to do here
    }

    void RomanPotsReconstruction_factory::Process(const std::shared_ptr<const JEvent> &event) {


	std::vector<edm4eic::ReconstructedParticle*> outputRPTracks;

	auto converter = m_geoSvc->cellIDPositionConverter();

	auto rawhits =  event->Get<edm4hep::SimTrackerHit>("ForwardRomanPotHits");


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

        for (const auto h: rawhits) {

            auto cellID = h->getCellID();

	    //global --> local begins here -----

            auto gpos = converter->position(cellID);

	    // local positions
            //if (m_localDetElement.empty()) {
            auto volman = m_geoSvc->detector()->volumeManager();
            local = volman.lookupDetElement(cellID);
	    //}

            auto pos0 = local.nominal().worldToLocal(dd4hep::Position(gpos.x(), gpos.y(), gpos.z())); // hit position in local coordinates

	    //information is stored in cm, we need mm - multiply everything by 10 for now

	    if(!goodHit2 && 10*gpos.z() > 27099.0 && 10*gpos.z() < 28022.0){

		goodHitX[1] = 10*pos0.x();
		goodHitY[1] = 10*pos0.y();
		goodHitZ[1] = 10*gpos.z();
	    	goodHit2 = true;

	    }
	    if(!goodHit1 && 10*gpos.z() > 25099.0 && 10*gpos.z() < 26022.0){

		goodHitX[0] = 10*pos0.x();
		goodHitY[0] = 10*pos0.y();
		goodHitZ[0] = 10*gpos.z();
		goodHit1 = true;

	    }

	}// end loop over hits

        // NB:
        // This is a "dumb" algorithm - I am just checking the basic thing works with a simple single-proton test.
        // Will need to update and modify for generic # of hits for more complicated final-states.

        if (goodHit1 && goodHit2) {

            // extract hit, subtract orbit offset – this is to get the hits in the coordinate system of the orbit
            // trajectory -- should eventually be in local coordinates.
	    //
            double XL[2] = {goodHitX[0], goodHitX[1]};
            double YL[2] = {goodHitY[0], goodHitY[1]};

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

            float prec[3] = {static_cast<float>((p * rsx) / norm), static_cast<float>((p * rsy) / norm),
                                   static_cast<float>(p / norm)};

	    float refPoint[3] = {static_cast<float>(goodHitX[0]), static_cast<float>(goodHitY[0]), static_cast<float>(goodHitZ[0])};

            //----- end RP reconstruction code ------

            edm4eic::MutableReconstructedParticle rpTrack;
            rpTrack.setType(0);
            rpTrack.setMomentum({prec});
            rpTrack.setEnergy(std::hypot(edm4eic::magnitude(rpTrack.getMomentum()), .938272));
            rpTrack.setReferencePoint({refPoint});
            rpTrack.setCharge(1);
            rpTrack.setMass(.938272);
            rpTrack.setGoodnessOfPID(1.);
            rpTrack.setPDG(2212);
            //rpTrack.covMatrix(); // @TODO: Errors
            outputRPTracks.push_back(new edm4eic::ReconstructedParticle(rpTrack));


        } // END matrix reco

	Set(outputRPTracks);

    }

}
