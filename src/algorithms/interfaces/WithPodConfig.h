// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_IWITHCONFIG_H
#define EICRECON_IWITHCONFIG_H
namespace eicrecon {

    /**
     * Small helper class that brings common functions interface for classes that have POD type config
     * @tparam ConfigT
     *
     * @example:
     *
     */
    template<typename ConfigT>
    class WithPodConfig {
    public:
        using ConfigType = ConfigT;

        /// Get a configuration to be changed
        ConfigT& getConfig() {return m_cfg;}

        /// Sets a configuration (config is properly copyible)
        ConfigT& applyConfig(ConfigT cfg) { m_cfg = cfg; return m_cfg;}

    protected:
        /** configuration parameters **/
        ConfigT m_cfg;
    };
}
#endif //EICRECON_IWITHCONFIG_H
