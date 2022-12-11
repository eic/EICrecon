#!/usr/bin/env ruby

# check for potentially unused params in reco_flags.py (requires ripgrep)

# collections (tags) from reco_flags.py
colls = [
  "B0ECalClusters",
  "B0ECalIslandProtoClusters",
  "B0ECalRawHits",
  "B0ECalRecHits",
  "BarrelTrackerHit",
  "BarrelTrackerRawHit",
  "BarrelVertexHit",
  "BarrelVertexRawHit",
  "CentralCKFTrajectories",
  "DRICHRawHits",
  "EcalBarrelImagingClusters",
  "EcalBarrelImagingMergedClusters",
  "EcalBarrelImagingProtoClusters",
  "EcalBarrelImagingRawHits",
  "EcalBarrelImagingRecHits",
  "EcalBarrelScFiClusters",
  "EcalBarrelscFiMergedHits",
  "EcalBarrelScFiProtoClusters",
  "EcalBarrelScFiRawHits",
  "EcalBarrelScFiRecHits",
  "EcalBarrelSciGlassClusters",
  "EcalBarrelSciGlassProtoClusters",
  "EcalBarrelSciGlassRawHits",
  "EcalBarrelSciGlassRecHits",
  "EcalBarrelSciGlassTruthClusters",
  "EcalEndcapNClusters",
  "EcalEndcapNIslandProtoClusters",
  "EcalEndcapNRawHits",
  "EcalEndcapNRecHits",
  "EcalEndcapNTruthClusters",
  "EcalEndcapPClusters",
  "EcalEndcapPInsertClusters",
  "EcalEndcapPInsertIslandProtoClusters",
  "EcalEndcapPInsertRawHits",
  "EcalEndcapPInsertRecHits",
  "EcalEndcapPInsertTruthClusters",
  "EcalEndcapPIslandProtoClusters",
  "EcalEndcapPRawHits",
  "EcalEndcapPRecHits",
  "EcalEndcapPTruthClusters",
  "EndcapTrackerHit",
  "EndcapTrackerRawHit",
  "ForwardRomanPotParticles",
  "ForwardRomanPotRawHits",
  "ForwardRomanPotRecHits",
  "GeneratedParticles",
  "HcalBarrelClusters",
  "HcalBarrelIslandProtoClusters",
  "HcalBarrelMergedHits",
  "HcalBarrelRawHits",
  "HcalBarrelRecHits",
  "HcalBarrelTruthClusters",
  "HcalEndcapNClusters",
  "HcalEndcapNIslandProtoClusters",
  "HcalEndcapNMergedHits",
  "HcalEndcapNRawHits",
  "HcalEndcapNRecHits",
  "HcalEndcapNTruthClusters",
  "HcalEndcapPClusters",
  "HcalEndcapPInsertClusters",
  "HcalEndcapPInsertIslandProtoClusters",
  "HcalEndcapPInsertRawHits",
  "HcalEndcapPInsertRecHits",
  "HcalEndcapPInsertTruthClusters",
  "HcalEndcapPIslandProtoClusters",
  "HcalEndcapPMergedHits",
  "HcalEndcapPRawHits",
  "HcalEndcapPRecHits",
  "HcalEndcapPTruthClusters",
  "MPGDTrackerHit",
  "MPGDTrackerRawHit",
  "TOFBarrelRawHit",
  "TOFBarrelTrackerHit",
  "TOFEndcapRawHit",
  "TOFEndcapTrackerHit",
  "ZDCEcalClusters",
  "ZDCEcalIslandProtoClusters",
  "ZDCEcalRawHits",
  "ZDCEcalRecHits",
  "ZDCEcalTruthClusters",
].sort

# use `ripgrep` (`rg`) to count number of c++ files mentioning each collection
unused = []
colls.each do |coll|
  num_found = `rg -tcpp -w -l '#{coll}' | wc -l`.chomp
  warning = ''
  if num_found.to_i == 0
    warning = '<--------- WARNING'
    unused << coll
  end
  puts "#{coll} found in #{num_found} cpp files #{warning}"
end

puts "\nPotentially unused parameters in `reco_flags.py`:"
unused.each do |coll|
  system "grep --color -w #{coll} src/tools/default_flags_table/reco_flags.py"
end
