
#include <optional>

#include <edm4eic/TrackCollection.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>


template <template <typename, typename> typename FuncT, typename RetValT, typename ... ArgsT>
std::optional<RetValT> CallWithPODIOType(std::string podio_typename, ArgsT... args) {
    if (podio_typename == "edm4eic::Track") {
        auto helper = FuncT<edm4eic::Track,edm4eic::TrackCollection>();
        return helper(std::forward<ArgsT>(args)...);
    }
    if (podio_typename == "edm4eic::CalorimeterHit") {
        auto helper = FuncT<edm4eic::CalorimeterHit,edm4eic::CalorimeterHitCollection>();
        return helper(std::forward<ArgsT>(args)...);
    }
    if (podio_typename == "edm4eic::Cluster") {
        auto helper = FuncT<edm4eic::Cluster,edm4eic::ClusterCollection>();
        return helper(std::forward<ArgsT>(args)...);
    }
    if (podio_typename == "edm4eic::SimTrackerHit") {
        auto helper = FuncT<edm4hep::SimTrackerHit,edm4hep::SimTrackerHitCollection>();
        return helper(std::forward<ArgsT>(args)...);
    }
    return std::nullopt;
}
