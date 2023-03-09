// Created by Dmitry Romanov -- edited by Alex Jentsch to do Roman Pots reconstruction
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include "RomanPotsReconstruction_factory.h"
#include <global/digi/SiliconTrackerDigi_factory.h>
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/string/StringHelpers.h"

namespace eicrecon {
    
	
    RomanPotsReconstruction_factory::RomanPotsReconstruction_factory(){ SetTag("ForwardRomanPotRecParticle"); }	
	
    void RomanPotsReconstruction_factory::Init() {
	// This prefix will be used for parameters
	//std::string plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
	//std::string param_prefix = plugin_name+ ":" + GetTag();

	// Create plugin level sub-log
	//m_log = spdlog::stdout_color_mt("RomanPotsReconstruction_factory");

	// Ask service locator for parameter manager. We want to get this plugin parameters.
	//auto pm = this->GetApplication()->GetJParameterManager();

	//pm->SetDefaultParameter(param_prefix + ":verbose", m_verbose, "verbosity: 0 - none, 1 - default, 2 - debug, 3 - trace");
	//pm->SetDefaultParameter(param_prefix + ":InputTags", m_input_tags, "Input data tag name");

        auto app = GetApplication(); //FIXME: What is this actually doing?

	m_log = app->GetService<Log_service>()->logger("ForwardRomanPotRecParticle");

		//-------------------------------------------------------------------------
		//----from FarForwardParticles.h  -----------------------------------------
		//-------------------------------------------------------------------------

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
    

	    //m_roman_pot_reco_algo.init();
    
    }//init

    void RomanPotsReconstruction_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void RomanPotsReconstruction_factory::Process(const std::shared_ptr<const JEvent> &event) {
	    // Now we check that user provided an input names
	    //std::vector<std::string> &input_tags = m_input_tags;

	    std::vector<edm4eic::ReconstructedParticle*> outputRPTracks;

	    //if(input_tags.empty()) {
	    //    input_tags = GetDefaultInputTags();
	   // }

		//-------------------------------------------------------------------------
		//----from FarForwardParticles.h  -----------------------------------------
		//-------------------------------------------------------------------------

    	// See Wouter's example to extract local coordinates CalorimeterHitReco.cpp
    	// includes DDRec/CellIDPositionConverter.here
    	// See tutorial
    	// auto converter = m_GeoSvc ....
    	// https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/CalorimeterHitReco.cpp - line 200
    	// include the Eigen libraries, used in ACTS, for the linear algebra.

        

        //auto &rawhits = m_inputHits;
	auto rawhits =  event->Get<edm4eic::TrackerHit>("ForwardRomanPotHits");
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

        for (const auto h: rawhits) {

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

            float prec[3] = {static_cast<float>(p * rsx / norm), static_cast<float>(p * rsy / norm),
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
            rpTrack.setPDG(2212);
            //rpTrack.covMatrix(); // @TODO: Errors
            outputRPTracks.push_back(new edm4eic::ReconstructedParticle(rpTrack));

	    //Set(outputRPTracks);

        } // end enough hits for matrix reco

	Set(outputRPTracks);

        return;
    }//process
	
	//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------

	/*
	    // Produce track parameters out of MCParticles
	    std::vector<edm4eic::ReconstructedParticle*> results;
	    for(auto rp_digi_hit: rp_digi_hits) {

	        // Status check for hits
			//we REALLY need to send the full hit collection to the algorithm
	        //if(rp_digi_hits->getGeneratorStatus() != 1 ) continue;

	        // Do conversion
	        auto result = m_roman_pot_reco_algo.produce(mc_particle);
	        results.push_back(result);

	        // >oO debug output
	        if(m_log->level() <= spdlog::level::debug) {
	            const auto p = std::hypot(result->getMomentum().x, result->getMomentum().y, result->getMomentum().z);
	            m_log->debug("Roman Pots raw hits produced track with ");
	            m_log->debug("   p =  {} GeV\"", p);
	            m_log->debug("   pT = {}", p.Pt());
	        }
	    }

	    Set(results);
	*/
	//}

	//Set(outputRPTracks);

} // eicrecon
