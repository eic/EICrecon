add_subdirectory(BEMC)
add_subdirectory(EEMC)
add_subdirectory(FEMC)
add_subdirectory(EHCAL)
add_subdirectory(BHCAL)
add_subdirectory(FHCAL)
add_subdirectory(ZDC)
add_subdirectory(B0ECAL)

# Tracking detectors
add_subdirectory(BTRK) # Barrel tracker
add_subdirectory(BVTX) # Barrel vertex
add_subdirectory(ECTRK) # EndCap tracker
add_subdirectory(MPGD) # MPDG tracker
add_subdirectory(RPOTS) # Roman Pots
add_subdirectory(FOFFMTRK) # OffMom tracker
add_subdirectory(ECTOF) # Endcap TOF
add_subdirectory(BTOF) # Barrel TOF
add_subdirectory(B0TRK) # B0 tracker
add_subdirectory(LOWQ2) # LOWQ2 Tagger
add_subdirectory(LUMISPECCAL) # Lumi spectrometer calorimeter

# PID detectors
add_subdirectory(DIRC)
# Will cover dRICH and pfRICH as well as sandbox detectors (FRICH and BRICH);
add_subdirectory(RICH-IRT)
