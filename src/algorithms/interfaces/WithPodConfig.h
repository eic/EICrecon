// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
namespace eicrecon {

/**
     * This struct might be used for factories that has no underlying config class,
     * for example:
     *    WithPodConfig<NoConfig>
     */
struct NoConfig {};

/**
     * Small helper class that brings common functions interface for classes that have POD type config
     * @tparam ConfigT
     *
     * @example:
     *
     */
template <typename ConfigT = NoConfig> class WithPodConfig {
public:
  using ConfigType = ConfigT;

  /// Get a configuration to be changed
  ConfigT& getConfig() { return m_cfg; }

  /// Sets a configuration (config is properly copyible)
  ConfigT& applyConfig(ConfigT cfg) {
    m_cfg = cfg;
    return m_cfg;
  }

protected:
  /** configuration parameters **/
  ConfigT m_cfg;
};
} // namespace eicrecon
