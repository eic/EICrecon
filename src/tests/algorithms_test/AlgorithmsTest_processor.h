// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_ALGORITHMSTEST_PROCESSOR_H
#define EICRECON_ALGORITHMSTEST_PROCESSOR_H

#include <JANA/JFactoryGenerator.h>
#include <JANA/JEventProcessor.h>
#include <JANA/JEventProcessorSequentialRoot.h>
#include <JANA/JEvent.h>

#include <services/log/Log_service.h>
#include <services/integration/Algorithms_service.h>

#include <edm4hep/SimCalorimeterHit.h>
#include <edm4eic/ProtoCluster.h>
#include <edm4eic/Cluster.h>

#include <algorithms/calorimetry/ClusterRecoCoG.h>



class AlgorithmsTest_processor:public JEventProcessor
{
public:
    explicit AlgorithmsTest_processor(JApplication *app):
            JEventProcessor(app),
            m_algo("ClusterRecoCoG")
    {

    };

    ~AlgorithmsTest_processor() override = default;

    //----------------------------
    // Init
    //
    // This is called once before the first call to the Process method
    // below. You may, for example, want to open an output file here.
    // Only one thread will call this.
    void Init() override {

        try {
            auto app = GetApplication();

            auto algo_svc = app->GetService<eicrecon::Algorithms_service>();
            auto &property_map = m_algo.getProperties();

            for (auto &[property_name, property]: property_map) {
                fmt::print("{}\n", property_name);
                //app->SetDefaultParameter(std::string(property_name), property.get(), "hahaha");
            }
            m_algo.init();
        }
        catch (std::exception &ex) {
            throw JException(ex.what());
        }

    }

    //----------------------------
    // Process
    //
    // This is called for every event. Multiple threads may call this
    // simultaneously. If you write something to an output file here
    // then make sure to protect it with a mutex or similar mechanism.
    // Minimize what is done while locked since that directly affects
    // the multi-threaded performance.
    void Process(const std::shared_ptr<const JEvent>& event) override {
        //auto proto_clusters = event->Get<edm4eic::ProtoCluster>("EcalBarrelTruthProtoClusters");
        auto proto_clusters_vec = event->Get<edm4eic::ProtoCluster>("EcalBarrelIslandProtoClusters");
        auto sim_hits = event->Get<edm4hep::SimCalorimeterHit>("EcalBarrelRecHits");
        //cluster = new edm4eic::ProtoCluster()
        // Copy from vectors to collections
        edm4eic::ProtoClusterCollection proto_clusters_col;
        edm4hep::SimCalorimeterHitCollection sim_hits_col;

        for(auto proto_cluster: proto_clusters_vec) {
            proto_clusters_col.push_back(*proto_cluster);
        }

        for(auto sim_hit_ptr: sim_hits) {
            sim_hits_col.push_back(*sim_hit_ptr);
        }

        edm4eic::ClusterCollection out_clusters;
        edm4eic::MCRecoClusterParticleAssociationCollection out_assoc;
        //std::optional<edm4eic::MCRecoClusterParticleAssociationCollection> out_assoc;
        const std::tuple<edm4eic::ProtoClusterCollection*, edm4hep::SimCalorimeterHitCollection*> input(&proto_clusters_col, &sim_hits_col);
        auto output = std::tuple<gsl::not_null<edm4eic::ClusterCollection *>, edm4eic::MCRecoClusterParticleAssociationCollection *>(&out_clusters, &out_assoc);

        m_algo.process(input, output);

        for(auto reco_cluster: *std::get<0>(output)){
            fmt::print("{}\n", reco_cluster.getEnergy());
        }
    }

    //----------------------------
    // Finish
    //
    // This is called once after all events have been processed. You may,
    // for example, want to close an output file here.
    // Only one thread will call this.
    void Finish() override {

    }

private:
    algorithms::calorimetry::ClusterRecoCoG m_algo;
    std::shared_ptr<spdlog::logger> m_log;
    TDirectory *m_dir_main;
};


#endif //EICRECON_ALGORITHMSTEST_PROCESSOR_H
