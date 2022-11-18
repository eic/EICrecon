from dd4hep_pgun_simulation import *

if __name__ == "__main__":

    should_upload = True

    for detector_config in ["epic_arches", "epic_brycecanyon"]:

        # 1 electron
        cfg = SimConfig()
        cfg.detector_config = detector_config
        cfg.particle = "e-"
        cfg.events_num = 10
        run_simulation(cfg, should_upload)

        cfg.events_num = 100
        run_simulation(cfg, should_upload)

        cfg.events_num = 1000
        run_simulation(cfg, should_upload)

        cfg.events_num = 10000
        run_simulation(cfg, should_upload)

        # 5 electrons

        # 100 events
        cfg = SimConfig()
        cfg.detector_config = detector_config
        cfg.particle = "e-"
        cfg.multiplicity = 5
        cfg.events_num = 100
        run_simulation(cfg, should_upload)

        # 2000 events
        cfg.events_num = 2000
        run_simulation(cfg, should_upload)

        # Now for pi+
        cfg = SimConfig()
        cfg.particle = "pi+"
        cfg.detector_config = detector_config
        cfg.events_num = 10
        run_simulation(cfg, should_upload)

        cfg.events_num = 100
        run_simulation(cfg, should_upload)

        cfg.events_num = 1000
        run_simulation(cfg, should_upload)

        cfg.events_num = 10000
        run_simulation(cfg, should_upload)