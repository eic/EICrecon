// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dmitry Kalinkin

#pragma once

#include <algorithms/service.h>
#include <memory>

class ActsGeometryProvider;

namespace algorithms {

class ActsSvc : public Service<ActsSvc> {
  public:
    virtual void init(std::shared_ptr<const ActsGeometryProvider> provider = nullptr) {
      m_acts_geometry_provider = provider;
    };

    std::shared_ptr<const ActsGeometryProvider> acts_geometry_provider() {
      return m_acts_geometry_provider;
    }

  protected:
    std::shared_ptr<const ActsGeometryProvider> m_acts_geometry_provider{nullptr};

  ALGORITHMS_DEFINE_SERVICE(ActsSvc)
};

}
