import os
from pyjano.jana import Jana
from eicrecon_path import cmake_source_root     # where this eicrecon is installed

if __name__ == "__main__":
    jana = Jana()
    jana.exec_path = f"{cmake_source_root}/bin/eicrecon"
    print(os.path.abspath(f"{cmake_source_root}/plugins"))
    jana.configure_plugin_paths([f"{cmake_source_root}/plugins"])
    jana.plugin("BTRK")
    jana.plugin("data_flow_test")
    jana.source("/home/romanov/work/data/athena_py8_5x41_mq2-10_beameff_ip6_hidiv_100ev.edm4hep.root")
    jana.run(retval_raise=True)
