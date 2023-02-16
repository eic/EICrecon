// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng

/*  General PhotoMultiplier Digitization
 *
 *  Apply the given quantum efficiency for photon detection
 *  Converts the number of detected photons to signal amplitude
 *
 *  Author: Chao Peng (ANL)
 *  Date: 10/02/2020
 */

//Ported by Thomas Britton (JLab)


#include "PhotoMultiplierHitDigi.h"

#include <JANA/JEvent.h>

//------------------------
// AlgorithmInit
//------------------------
void eicrecon::PhotoMultiplierHitDigi::AlgorithmInit(dd4hep::Detector *detector, std::shared_ptr<spdlog::logger>& logger)
{
    // services
    m_cellid_converter = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(*detector);
    m_log=logger;

    // print the configuration parameters
    m_cfg.Print(m_log, spdlog::level::debug);

    // random number generators
    m_random.SetSeed(m_cfg.seed);
    m_rngNorm = [&](){
        return m_random.Gaus(0., 1.0);
    };
    m_rngUni = [&](){
        return m_random.Uniform(0., 1.0);
    };
    //auto randSvc = svc<IRndmGenSvc>("RndmGenSvc", true);
    auto sc1 = m_rngUni;//m_rngUni.initialize(randSvc, Rndm::Flat(0., 1.));
    auto sc2 = m_rngNorm;//m_rngNorm.initialize(randSvc, Rndm::Gauss(0., 1.));
    //if (!sc1.isSuccess() || !sc2.isSuccess()) {
    if (!sc1 || !sc2) {
        m_log->error("Cannot initialize random generator!");
        japp->Quit();
    }

    // initialize quantum efficiency table
    qe_init();
}



//------------------------
// AlgorithmChangeRun
//------------------------
void eicrecon::PhotoMultiplierHitDigi::AlgorithmChangeRun() {
    /// This is automatically run before Process, when a new run number is seen
    /// Usually we update our calibration constants by asking a JService
    /// to give us the latest data for this run number
}

//------------------------
// AlgorithmProcess
//------------------------
std::vector<edm4eic::RawPMTHit*>
eicrecon::PhotoMultiplierHitDigi::AlgorithmProcess(std::vector<const edm4hep::SimTrackerHit*>& sim_hits) {

        m_log->trace("{:=^70}"," call PhotoMultiplierHitDigi::AlgorithmProcess ");
        struct HitData {
          uint32_t npe;
          double signal;
          double time;
          dd4hep::Position pos;
        };
        std::unordered_map<uint64_t, std::vector<HitData>> hit_groups;
        // collect the photon hit in the same cell
        // calculate signal
        for(const auto& ahit : sim_hits) {
            auto edep_eV = ahit->getEDep() * 1e9; // [GeV] -> [eV] // FIXME: use common unit converters, when available
            // m_log->trace("{:-<60}","Simulation Hit");
            // m_log->trace("  {:>20}: {}\n", "EDep", edep_eV);

            // overall safety factor
            if (m_rngUni() > m_cfg.safetyFactor) continue;

            // quantum efficiency
            if (!qe_pass(edep_eV, m_rngUni())) continue;

            // pixel gap cuts
            // FIXME: generalize; this assumes the segmentation is `CartesianGridXY`
            auto id = ahit->getCellID();
            dd4hep::Position pos_pixel, pos_hit;
            if(m_cfg.enablePixelGaps) {
              pos_pixel = get_sensor_local_position( id, m_cellid_converter->position(id) );
              pos_hit   = get_sensor_local_position( id, vec2pos(ahit->getPosition())     );
              if( std::abs( pos_hit.x()/dd4hep::mm - pos_pixel.x()/dd4hep::mm ) > m_cfg.pixelSize/2 ||
                  std::abs( pos_hit.y()/dd4hep::mm - pos_pixel.y()/dd4hep::mm ) > m_cfg.pixelSize/2
                ) continue;
            }

            // cell time, signal amplitude
            double time = ahit->getTime();//ahit->getMCParticle().getTime();
            double amp = m_cfg.speMean + m_rngNorm()*m_cfg.speError;

            // group hits
            auto it = hit_groups.find(id);
            if (it != hit_groups.end()) {
                size_t i = 0;
                for (auto git = it->second.begin(); git != it->second.end(); ++git, ++i) {
                    if (std::abs(time - git->time) <= (m_cfg.hitTimeWindow)) {
                        git->npe += 1;
                        git->signal += amp;
                        break;
                    }
                }
                // no hits group found
                if (i >= it->second.size()) {
                    it->second.emplace_back(HitData{1, amp + m_cfg.pedMean + m_cfg.pedError*m_rngNorm(), time, pos_hit});
                }
            } else {
                hit_groups[id] = {HitData{1, amp + m_cfg.pedMean + m_cfg.pedError*m_rngNorm(), time, pos_hit}};
            }
        }

        // print `hit_groups`
        if(m_log->level() <= spdlog::level::trace)
          for(auto &[id,hitVec] : hit_groups)
            for(auto &hit : hitVec)
              m_log->trace("hit_group: pixel id={:#018x} -> npe={} signal={:<5g} time={:<5g}", id, hit.npe, hit.signal, hit.time);

        // build raw hits
        std::vector<edm4eic::RawPMTHit*> raw_hits;
        for (auto &it : hit_groups) {
            for (auto &data : it.second) {
                edm4eic::RawPMTHit* hit = new edm4eic::RawPMTHit{
                  it.first,
                  static_cast<uint32_t>(data.signal),
                  static_cast<uint32_t>(data.time/m_cfg.timeStep)
                  //,pos2vec(data.pos) // TEST gap cuts; requires member `edm4hep::Vector3d position` in data model datatype
                };
                raw_hits.push_back(hit);
            }
        }
        return raw_hits;

}

void  eicrecon::PhotoMultiplierHitDigi::qe_init()
{
        // get quantum efficiency table
        qeff.clear();
        for(const auto &[wl,qe] : m_cfg.quantumEfficiency)
          qeff.push_back({ 1239.84 / wl, qe }); // convert wavelength [nm] -> energy [eV]

        // sort quantum efficiency data first
        std::sort(qeff.begin(), qeff.end(),
            [] (const std::pair<double, double> &v1, const std::pair<double, double> &v2) {
                return v1.first < v2.first;
            });

        // print the table
        m_log->debug("{:-^60}"," Quantum Efficiency vs. Energy ");
        for(auto& [en,qe] : qeff)
          m_log->debug("  {:>10.4} {:<}",en,qe);
        m_log->trace("{:=^60}","");

        // sanity checks
        if (qeff.empty()) {
            qeff = {{2.6, 0.3}, {7.0, 0.3}};
            m_log->warn("Invalid quantum efficiency data provided, using default values {} {:.2f} {} {:.2f} {} {:.2f} {} {:.2f} {}","{{", qeff.front().first, ",", qeff.front().second, "},{",qeff.back().first,",",qeff.back().second,"}}");
        }
        if (qeff.front().first > 3.0) {
            m_log->warn("Quantum efficiency data start from {:.2f} {}", qeff.front().first, " eV, maybe you are using wrong units?");
        }
        if (qeff.back().first < 3.0) {
            m_log->warn("Quantum efficiency data end at {:.2f} {}", qeff.back().first, " eV, maybe you are using wrong units?");
        }
}


template<class RndmIter, typename T, class Compare> RndmIter  eicrecon::PhotoMultiplierHitDigi::interval_search(RndmIter beg, RndmIter end, const T &val, Compare comp) const
{
        // special cases
        auto dist = std::distance(beg, end);
        if ((dist < 2) || (comp(*beg, val) > 0) || (comp(*std::prev(end), val) < 0)) {
            return end;
        }
        auto mid = std::next(beg, dist / 2);

        while (mid != end) {
            if (comp(*mid, val) == 0) {
                return mid;
            } else if (comp(*mid, val) > 0) {
                end = mid;
            } else {
                beg = std::next(mid);
            }
            mid = std::next(beg, std::distance(beg, end)/2);
        }

        if (mid == end || comp(*mid, val) > 0) {
            return std::prev(mid);
        }
        return mid;
}

bool  eicrecon::PhotoMultiplierHitDigi::qe_pass(double ev, double rand) const
{
        auto it = interval_search(qeff.begin(), qeff.end(), ev,
                    [] (const std::pair<double, double> &vals, double val) {
                        return vals.first - val;
                    });

        if (it == qeff.end()) {
            // m_log->warn("{} eV is out of QE data range, assuming 0\% efficiency",ev);
            return false;
        }

        double prob = it->second;
        auto itn = std::next(it);
        if (itn != qeff.end() && (itn->first - it->first != 0)) {
            prob = (it->second*(itn->first - ev) + itn->second*(ev - it->first)) / (itn->first - it->first);
        }

        // m_log->trace("{} eV, QE: {}\%",ev,prob*100.);
        return rand <= prob;
}


// transform global position `pos` to sensor `id` frame position
// IMPORTANT NOTE: this has only been tested for the dRICH; if you use it, test it carefully...
// FIXME: here be dragons...
dd4hep::Position eicrecon::PhotoMultiplierHitDigi::get_sensor_local_position(uint64_t id, dd4hep::Position pos) {

  // get the VolumeManagerContext for this sensitive detector
  auto context = m_cellid_converter->findContext(id);

  // transformation vector buffers
  double xyz_l[3], xyz_e[3], xyz_g[2];
  double pv_g[3], pv_l[3];

  // get sensor position w.r.t. its parent
  auto sensor_elem = context->element;
  sensor_elem.placement().position().GetCoordinates(xyz_l);

  // convert sensor position to global position (cf. `CellIDPositionConverter::positionNominal()`)
  const auto& volToElement = context->toElement();
  volToElement.LocalToMaster(xyz_l, xyz_e);
  const auto& elementToGlobal = sensor_elem.nominal().worldTransformation();
  elementToGlobal.LocalToMaster(xyz_e, xyz_g);
  dd4hep::Position pos_sensor;
  pos_sensor.SetCoordinates(xyz_g);

  // get the position vector of `pos` w.r.t. the sensor position `pos_sensor`
  dd4hep::Direction pos_pv = pos - pos_sensor;

  // then transform it to the sensor's local frame
  pos_pv.GetCoordinates(pv_g);
  volToElement.MasterToLocalVect(pv_g, pv_l);
  dd4hep::Position pos_transformed;
  pos_transformed.SetCoordinates(pv_l);

  // trace log
  if(m_log->level() <= spdlog::level::trace) {
    m_log->trace("pixel hit on cellID={:#018x}",id);
    auto print_pos = [&] (std::string name, dd4hep::Position p) {
      m_log->trace("  {:>30} x={:.2f} y={:.2f} z={:.2f} [mm]: ", name, p.x()/dd4hep::mm,  p.y()/dd4hep::mm,  p.z()/dd4hep::mm);
    };
    print_pos("input position",  pos);
    print_pos("sensor position", pos_sensor);
    print_pos("output position", pos_transformed);
    // auto dim = m_cellid_converter->cellDimensions(id);
    // for (size_t j = 0; j < std::size(dim); ++j)
    //   m_log->trace("   - dimension {:<5} size: {:.2}",  j, dim[j]);
  }

  return pos_transformed;
}
