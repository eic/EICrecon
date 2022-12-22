// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _TrackPoint_factory_HcalEndcapNProjections_h_
#define _TrackPoint_factory_HcalEndcapNProjections_h_

#include <Acts/Surfaces/DiscSurface.hpp>
#include <JANA/JFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <algorithms/tracking/TrackPropagation.h>


class TrackPoint_factory_HcalEndcapNProjections :
        public JFactoryT<edm4eic::TrackPoint>,
        public eicrecon::SpdlogMixin<TrackPoint_factory_HcalEndcapNProjections>
        {

public:
    //------------------------------------------
    // Constructor
    TrackPoint_factory_HcalEndcapNProjections(){
        SetTag("HcalEndcapNProjections");
    }

    //------------------------------------------
    // Init
    void Init() override;

    //------------------------------------------
    // ChangeRun
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override {};

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override;

private:
    /// Tracking propagation algorithm
    eicrecon::TrackPropagation m_propagation_algo;

    /// A surface to propagate to
    std::shared_ptr<Acts::DiscSurface> m_hcal_surface;
};

#endif // _Cluster_factory_HcalBarrelClusters_h_
