import numpy as np
from pprint import pprint

# using loadtxt()
arr = np.genfromtxt("/home/romanov/Downloads/calibration_constants - reco_flags_dump.csv",
                    delimiter=",", dtype=str, filling_values="")
pprint(arr)