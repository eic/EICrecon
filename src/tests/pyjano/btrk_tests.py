from pyjano.jana import Jana
from eicrecon_path import cmake_source_root     # where this eicrecon is installed

if __name__ == "__main__":
    jana = Jana()
    jana.exec_path = f"{cmake_source_root}/bin/eicrecon"
    # jana.configure_plugin_paths()
    jana.run(retval_raise=True)

