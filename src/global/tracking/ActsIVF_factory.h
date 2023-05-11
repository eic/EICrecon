// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>

#include <algorithms/tracking/ActsIVF.h>

#include <edm4eic/TrackParameters.h>
#include <edm4eic/Vertex.h>
#include <edm4eic/VertexCollection.h>
#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>

namespace eicrecon {

class ActsIVF_factory : public JChainFactoryT<edm4eic::Vertex, IVFConfig>,
                        public SpdlogMixin<ActsIVF_factory> {

public:
  explicit ActsIVF_factory(std::vector<std::string> default_input_tags, IVFConfig cfg)
      : JChainFactoryT<edm4eic::Vertex, IVFConfig>(std::move(default_input_tags), cfg) {}

  /** One time initialization **/
  void Init() override;

  /** On run change preparations **/
  void ChangeRun(const std::shared_ptr<const JEvent>& event) override;

  /** Event by event processing **/
  void Process(const std::shared_ptr<const JEvent>& event) override;

private:
  eicrecon::ActsIVF m_vertexing_algo; /// Proxy tracking algorithm
};

} // namespace eicrecon
