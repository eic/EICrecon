import subprocess
import datetime

from dd4hep_pgun_simulation import SimConfig, run_simulation

if __name__ == "__main__":

    should_upload = False

    # 1 electron
    cfg = SimConfig()
    cfg.multiplicity = 4
    cfg.particle = "pi-"
    # cfg.events_num = 10
    # run_simulation(cfg, should_upload)
    #
    # cfg.events_num = 100
    # run_simulation(cfg, should_upload)
    cfg.detector_config = "epic_arches"

    cfg.events_num = 10000
    run_simulation(cfg, should_upload)
