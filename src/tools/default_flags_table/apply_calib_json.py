import os
import json

# substitute of 'from GaudiKernel.SystemOfUnits import eV, MeV, GeV, mm, cm, mrad'
from system_of_units import eV, MeV, GeV, mm, cm, mrad

detector_path = str(os.environ.get("DETECTOR_PATH", "."))
detector_name = str(os.environ.get("DETECTOR_CONFIG", "epic"))
detector_config = str(os.environ.get("DETECTOR_CONFIG", detector_name))
detector_version = str(os.environ.get("DETECTOR_VERSION", "main"))

# Detector features that affect reconstruction
has_ecal_barrel_scfi = False
if "epic" in detector_name and "imaging" in detector_config:
    has_ecal_barrel_scfi = True

if "PBEAM" in os.environ:
    ionBeamEnergy = str(os.environ["PBEAM"])
else:
    ionBeamEnergy = 100

# ZDC reconstruction calibrations
try:
    ffi_zdc_calibrations = "calibrations/ffi_zdc.json"
    with open(os.path.join(detector_path, ffi_zdc_calibrations)) as f:
        ffi_zdc_config = json.load(f)

        def ffi_zdc_cal_parse(ffi_zdc_cal):
            ffi_zdc_cal_sf = float(ffi_zdc_cal["sampling_fraction"])
            ffi_zdc_cal_cl_kwargs = {
                "minClusterCenterEdep": eval(ffi_zdc_cal["minClusterCenterEdep"]),
                "minClusterHitEdep": eval(ffi_zdc_cal["minClusterHitEdep"]),
                "localDistXY": [
                    eval(ffi_zdc_cal["localDistXY"][0]),
                    eval(ffi_zdc_cal["localDistXY"][1]),
                ],
                "splitCluster": bool(ffi_zdc_cal["splitCluster"]),
            }
            return ffi_zdc_cal_sf, ffi_zdc_cal_cl_kwargs

        ffi_zdc_ecal_sf, ffi_zdc_ecal_cl_kwargs = ffi_zdc_cal_parse(
            ffi_zdc_config["ffi_zdc_ecal"]
        )
        ffi_zdc_hcal_sf, ffi_zdc_hcal_cl_kwargs = ffi_zdc_cal_parse(
            ffi_zdc_config["ffi_zdc_hcal"]
        )
except (IOError, OSError):
    print(f"Using default ffi_zdc calibrations; {ffi_zdc_calibrations} not found.")
    ffi_zdc_ecal_sf = float(os.environ.get("FFI_ZDC_ECAL_SAMP_FRAC", 1.0))
    ffi_zdc_hcal_sf = float(os.environ.get("FFI_ZDC_HCAL_SAMP_FRAC", 1.0))

print("ffi_zdc_ecal_sf")
print(ffi_zdc_ecal_sf)
print("ffi_zdc_ecal_cl_kwargs")
print(ffi_zdc_ecal_cl_kwargs)
print("ffi_zdc_hcal_sf")
print(ffi_zdc_hcal_sf)
print("ffi_zdc_hcal_cl_kwargs")
print(ffi_zdc_hcal_cl_kwargs)

# RICH reconstruction
qe_data = [
    (1.0, 0.25),
    (7.5, 0.25),
]

# CAL reconstruction
# get sampling fractions from system environment variable
ci_ecal_sf = float(os.environ.get("CI_ECAL_SAMP_FRAC", 0.03))
cb_hcal_sf = float(os.environ.get("CB_HCAL_SAMP_FRAC", 0.038))
ci_hcal_sf = float(os.environ.get("CI_HCAL_SAMP_FRAC", 0.025))
ce_hcal_sf = float(os.environ.get("CE_HCAL_SAMP_FRAC", 0.025))

# input arguments from calibration file
with open(f"{detector_path}/calibrations/emcal_barrel_calibration.json") as f:
    calib_data = json.load(f)["electron"]

print(calib_data)

# input calorimeter DAQ info
calo_daq = {}
with open(f"{detector_path}/calibrations/calo_digi_default.json") as f:
    calo_config = json.load(f)
    ## add proper ADC capacity based on bit depth
    for sys in calo_config:
        cfg = calo_config[sys]
        calo_daq[sys] = {
            "dynamicRangeADC": eval(cfg["dynamicRange"]),
            "capacityADC": 2 ** int(cfg["capacityBitsADC"]),
            "pedestalMean": int(cfg["pedestalMean"]),
            "pedestalSigma": float(cfg["pedestalSigma"]),
        }
print(calo_daq)

img_barrel_sf = float(calib_data["sampling_fraction_img"])
scifi_barrel_sf = float(calib_data["sampling_fraction_scfi"])
