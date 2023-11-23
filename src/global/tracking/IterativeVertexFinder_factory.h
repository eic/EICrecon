// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <edm4eic/VertexCollection.h>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "IterativeVertexFinderConfig.h"
#include "algorithms/tracking/IterativeVertexFinder.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

class IterativeVertexFinder_factory
    : public JChainMultifactoryT<IterativeVertexFinderConfig>,
      public SpdlogMixin {

public:
  explicit IterativeVertexFinder_factory(
      std::string tag,
      const std::vector<std::string>& input_tags,
      const std::vector<std::string>& output_tags,
      IterativeVertexFinderConfig cfg)
  : JChainMultifactoryT<IterativeVertexFinderConfig>(std::move(tag), input_tags, output_tags, cfg) {
      DeclarePodioOutput<edm4eic::Vertex>(GetOutputTags()[0]);
  }

  /** One time initialization **/
  void Init() override;

  /** Event by event processing **/
  void Process(const std::shared_ptr<const JEvent>& event) override;

private:
  eicrecon::IterativeVertexFinder m_vertexing_algo; /// Proxy vertexing algorithm
};

} // namespace eicrecon
