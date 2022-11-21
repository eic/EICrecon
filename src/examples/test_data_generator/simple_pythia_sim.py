"""Runs pythia hepmc file through dd4hep simulation"""
import subprocess
from datetime import datetime

input_file = "/home/romanov/work/data/ccdis/pythia8CCDIS_10x100_minQ2-1000.hepmc"
output_file = "/home/romanov/work/data/ccdis/pythia8CCDIS_10x100_minQ2-1000_10000evt.edm4hep.root"
events_num = 10000
detector_config = "epic_arches"

sim_command = [
    f'ddsim',
    f'--compactFile=$DETECTOR_PATH/{detector_config}.xml',
    f'-N {events_num}',
    f'--random.seed=1',
    f'--inputFiles {input_file}',
    f'--outputFile={output_file}',
]

start_time = datetime.now()
print(f"Simulation start time: {start_time.strftime('%Y-%m-%d %H:%M:%S')}")
print(f"Simulation Command:\n    {sim_command}")

# run! run!
subprocess.run(" ".join(sim_command), shell=True, check=True)

# reporting end
end_time = datetime.now()
print(f"Simulation ended at: {end_time.strftime('%Y-%m-%d %H:%M:%S')}, duration {end_time - start_time}: ")




# [filename](_media/example.js ':include :type=code :fragment=demo')