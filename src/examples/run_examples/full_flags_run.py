desc = """
This script is a simple demonstration of how to run eicrecon with available digitization/reconstruction flag
Currently this script is created manually (TODO automatic generation) so if flags here is outdated, please file an issue

    python3 full_flags_run.py input_file.edm4hep.root output_name_no_ext

Script should successfully run and create files:

    output_name_no_ext.edm4eic.root    # with output flat tree
    output_name_no_ext.ana.root        # with histograms and other things filled by monitoring plugins

"""

import argparse

parser = argparse.ArgumentParser(description=desc)
parser.add_argument('input_file', help="Input file name")
parser.add_argument('output_base_name', help="Output files names (no file extensions here)")
args = parser.parse_args()

flags = [

]

# (!) NEXT WILL BE REPLACED BY TEMPLATE #
reco_parameters_flags = []




