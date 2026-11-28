# SPDX-License-Identifier: LGPL-3.0-or-later
"""global_pid_lut wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import meta, pid_lut

PLUGIN = "global_pid_lut"


def configure(app, plugin=PLUGIN):
    with app.plugin(plugin):
        app.add(pid_lut.PIDLookup, 'RICHEndcapNTruthSeededLUTPID',
                ['EventHeader', 'ReconstructedTruthSeededChargedWithoutPIDParticles', 'ReconstructedTruthSeededChargedWithoutPIDParticleLinks'],
                ['ReconstructedTruthSeededChargedWithPFRICHPIDParticles', 'ReconstructedTruthSeededChargedWithPFRICHPIDParticleLinks', 'ReconstructedTruthSeededChargedWithPFRICHPIDParticleAssociations', 'RICHEndcapNTruthSeededParticleIDs'],
                filename='calibrations/pfrich.lut',
                system='BackwardRICH_ID',
                pdg_values=[11, 211, 321, 2212],
                charge_values=[1],
                momentum_edges=[0.4, 0.8, 1.2, 1.6, 2.0, 2.4, 2.8, 3.2, 3.6, 4.0, 4.4, 4.8, 5.2, 5.6, 6.0, 6.4, 6.8, 7.2, 7.6, 8.0, 8.4, 8.8, 9.2, 9.6, 10.0, 10.4, 10.8, 11.2, 11.6, 12.0, 12.4, 12.8, 13.2, 13.6, 14.0, 14.4, 14.8, 15.2],
                polar_edges=[2.65, 2.6725, 2.695, 2.7175, 2.74, 2.7625, 2.785, 2.8075, 2.83, 2.8525, 2.875, 2.8975, 2.92, 2.9425, 2.965, 2.9875, 3.01, 3.0325, 3.055, 3.0775],
                azimuthal_binning=[0.0, 6.283185307179586, 0.05235987755982988],
                azimuthal_bin_centers_in_lut=True,
                momentum_bin_centers_in_lut=True,
                polar_bin_centers_in_lut=True,
                use_radians=True)
        app.add(pid_lut.PIDLookup, 'RICHEndcapNLUTPID',
                ['EventHeader', 'ReconstructedChargedWithoutPIDParticles', 'ReconstructedChargedWithoutPIDParticleLinks'],
                ['ReconstructedChargedWithPFRICHPIDParticles', 'ReconstructedChargedWithPFRICHPIDParticleLinks', 'ReconstructedChargedWithPFRICHPIDParticleAssociations', 'RICHEndcapNParticleIDs'],
                filename='calibrations/pfrich.lut',
                system='BackwardRICH_ID',
                pdg_values=[11, 211, 321, 2212],
                charge_values=[1],
                momentum_edges=[0.4, 0.8, 1.2, 1.6, 2.0, 2.4, 2.8, 3.2, 3.6, 4.0, 4.4, 4.8, 5.2, 5.6, 6.0, 6.4, 6.8, 7.2, 7.6, 8.0, 8.4, 8.8, 9.2, 9.6, 10.0, 10.4, 10.8, 11.2, 11.6, 12.0, 12.4, 12.8, 13.2, 13.6, 14.0, 14.4, 14.8, 15.2],
                polar_edges=[2.65, 2.6725, 2.695, 2.7175, 2.74, 2.7625, 2.785, 2.8075, 2.83, 2.8525, 2.875, 2.8975, 2.92, 2.9425, 2.965, 2.9875, 3.01, 3.0325, 3.055, 3.0775],
                azimuthal_binning=[0.0, 6.283185307179586, 0.05235987755982988],
                azimuthal_bin_centers_in_lut=True,
                momentum_bin_centers_in_lut=True,
                polar_bin_centers_in_lut=True,
                use_radians=True)
        app.add(pid_lut.PIDLookup, 'CombinedTOFTruthSeededLUTPID',
                ['EventHeader', 'ReconstructedTruthSeededChargedWithPFRICHPIDParticles', 'ReconstructedTruthSeededChargedWithPFRICHPIDParticleLinks'],
                ['ReconstructedTruthSeededChargedWithPFRICHTOFPIDParticles', 'ReconstructedTruthSeededChargedWithPFRICHTOFPIDParticleLinks', 'ReconstructedTruthSeededChargedWithPFRICHTOFPIDParticleAssociations', 'CombinedTOFTruthSeededParticleIDs'],
                filename='calibrations/tof.lut',
                system='BarrelTOF_ID',
                pdg_values=[11, 211, 321, 2212],
                charge_values=[1],
                momentum_edges=[0.0, 0.3, 0.6, 0.9, 1.2, 1.5, 1.8, 2.1, 2.4, 2.7, 3.0, 3.3, 3.6, 3.9, 4.2, 4.5, 4.8, 5.1, 5.4, 5.7, 6.0],
                polar_edges=[2.5, 10.95, 19.4, 27.85, 36.3, 44.75, 53.2, 61.65, 70.1, 78.55, 87.0, 95.45, 103.9, 112.35, 120.8, 129.25, 137.7, 146.15, 154.6],
                azimuthal_binning=[0.0, 360.0, 360.0],
                momentum_bin_centers_in_lut=True,
                polar_bin_centers_in_lut=True)
        app.add(pid_lut.PIDLookup, 'CombinedTOFLUTPID',
                ['EventHeader', 'ReconstructedChargedWithPFRICHPIDParticles', 'ReconstructedChargedWithPFRICHPIDParticleLinks'],
                ['ReconstructedChargedWithPFRICHTOFPIDParticles', 'ReconstructedChargedWithPFRICHTOFPIDParticleLinks', 'ReconstructedChargedWithPFRICHTOFPIDParticleAssociations', 'CombinedTOFParticleIDs'],
                filename='calibrations/tof.lut',
                system='BarrelTOF_ID',
                pdg_values=[11, 211, 321, 2212],
                charge_values=[1],
                momentum_edges=[0.0, 0.3, 0.6, 0.9, 1.2, 1.5, 1.8, 2.1, 2.4, 2.7, 3.0, 3.3, 3.6, 3.9, 4.2, 4.5, 4.8, 5.1, 5.4, 5.7, 6.0],
                polar_edges=[2.5, 10.95, 19.4, 27.85, 36.3, 44.75, 53.2, 61.65, 70.1, 78.55, 87.0, 95.45, 103.9, 112.35, 120.8, 129.25, 137.7, 146.15, 154.6],
                azimuthal_binning=[0.0, 360.0, 360.0],
                momentum_bin_centers_in_lut=True,
                polar_bin_centers_in_lut=True)
        app.add(pid_lut.PIDLookup, 'DIRCTruthSeededLUTPID',
                ['EventHeader', 'ReconstructedTruthSeededChargedWithPFRICHTOFPIDParticles', 'ReconstructedTruthSeededChargedWithPFRICHTOFPIDParticleLinks'],
                ['ReconstructedTruthSeededChargedWithPFRICHTOFDIRCPIDParticles', 'ReconstructedTruthSeededChargedWithPFRICHTOFDIRCPIDParticleLinks', 'ReconstructedTruthSeededChargedWithPFRICHTOFDIRCPIDParticleAssociations', 'DIRCTruthSeededParticleIDs'],
                filename='calibrations/hpdirc.lut.gz',
                system='BarrelDIRC_ID',
                pdg_values=[11, 211, 321, 2212],
                charge_values=[-1, 1],
                momentum_edges=[0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0, 3.2, 3.4, 3.6, 3.8, 4.0, 4.2, 4.4, 4.6, 4.8, 5.0, 5.2, 5.4, 5.6, 5.8, 6.0, 6.2, 6.4, 6.6, 6.8, 7.0, 7.2, 7.4, 7.6, 7.8, 8.0, 8.2, 8.4, 8.6, 8.8, 9.0, 9.2, 9.4, 9.6, 9.8, 10.0, 10.2],
                polar_edges=[25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0, 49.0, 50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0, 57.0, 58.0, 59.0, 60.0, 61.0, 62.0, 63.0, 64.0, 65.0, 66.0, 67.0, 68.0, 69.0, 70.0, 71.0, 72.0, 73.0, 74.0, 75.0, 76.0, 77.0, 78.0, 79.0, 80.0, 81.0, 82.0, 83.0, 84.0, 85.0, 86.0, 87.0, 88.0, 89.0, 90.0, 91.0, 92.0, 93.0, 94.0, 95.0, 96.0, 97.0, 98.0, 99.0, 100.0, 101.0, 102.0, 103.0, 104.0, 105.0, 106.0, 107.0, 108.0, 109.0, 110.0, 111.0, 112.0, 113.0, 114.0, 115.0, 116.0, 117.0, 118.0, 119.0, 120.0, 121.0, 122.0, 123.0, 124.0, 125.0, 126.0, 127.0, 128.0, 129.0, 130.0, 131.0, 132.0, 133.0, 134.0, 135.0, 136.0, 137.0, 138.0, 139.0, 140.0, 141.0, 142.0, 143.0, 144.0, 145.0, 146.0, 147.0, 148.0, 149.0, 150.0, 151.0, 152.0, 153.0, 154.0, 155.0, 156.0, 157.0, 158.0, 159.0, 160.0],
                azimuthal_binning=[0.0, 30.5, 0.5])
        app.add(pid_lut.PIDLookup, 'DIRCLUTPID',
                ['EventHeader', 'ReconstructedChargedWithPFRICHTOFPIDParticles', 'ReconstructedChargedWithPFRICHTOFPIDParticleLinks'],
                ['ReconstructedChargedWithPFRICHTOFDIRCPIDParticles', 'ReconstructedChargedWithPFRICHTOFDIRCPIDParticleLinks', 'ReconstructedChargedWithPFRICHTOFDIRCPIDParticleAssociations', 'DIRCParticleIDs'],
                filename='calibrations/hpdirc.lut.gz',
                system='BarrelDIRC_ID',
                pdg_values=[11, 211, 321, 2212],
                charge_values=[-1, 1],
                momentum_edges=[0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0, 3.2, 3.4, 3.6, 3.8, 4.0, 4.2, 4.4, 4.6, 4.8, 5.0, 5.2, 5.4, 5.6, 5.8, 6.0, 6.2, 6.4, 6.6, 6.8, 7.0, 7.2, 7.4, 7.6, 7.8, 8.0, 8.2, 8.4, 8.6, 8.8, 9.0, 9.2, 9.4, 9.6, 9.8, 10.0, 10.2],
                polar_edges=[25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0, 49.0, 50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0, 57.0, 58.0, 59.0, 60.0, 61.0, 62.0, 63.0, 64.0, 65.0, 66.0, 67.0, 68.0, 69.0, 70.0, 71.0, 72.0, 73.0, 74.0, 75.0, 76.0, 77.0, 78.0, 79.0, 80.0, 81.0, 82.0, 83.0, 84.0, 85.0, 86.0, 87.0, 88.0, 89.0, 90.0, 91.0, 92.0, 93.0, 94.0, 95.0, 96.0, 97.0, 98.0, 99.0, 100.0, 101.0, 102.0, 103.0, 104.0, 105.0, 106.0, 107.0, 108.0, 109.0, 110.0, 111.0, 112.0, 113.0, 114.0, 115.0, 116.0, 117.0, 118.0, 119.0, 120.0, 121.0, 122.0, 123.0, 124.0, 125.0, 126.0, 127.0, 128.0, 129.0, 130.0, 131.0, 132.0, 133.0, 134.0, 135.0, 136.0, 137.0, 138.0, 139.0, 140.0, 141.0, 142.0, 143.0, 144.0, 145.0, 146.0, 147.0, 148.0, 149.0, 150.0, 151.0, 152.0, 153.0, 154.0, 155.0, 156.0, 157.0, 158.0, 159.0, 160.0],
                azimuthal_binning=[0.0, 30.5, 0.5])
        app.add(meta.CollectionCollector_ReconstructedParticle_true, 'ReconstructedWithPFRICHTOFDIRCLOWQ2PIDChargedParticles',
                ['ReconstructedChargedWithPFRICHTOFDIRCPIDParticles', 'TaggerTrackerReconstructedParticles'],
                ['ReconstructedWithPFRICHTOFDIRCLOWQ2PIDChargedParticles'])
        app.add(meta.CollectionCollector_MCRecoParticleAssociation_true, 'ReconstructedChargedWithPFRICHTOFDIRCLOWQ2PIDParticleAssociations',
                ['ReconstructedChargedWithPFRICHTOFDIRCPIDParticleAssociations', 'TaggerTrackerReconstructedParticleAssociations'],
                ['ReconstructedChargedWithPFRICHTOFDIRCLOWQ2PIDParticleAssociations'])
        app.add(meta.CollectionCollector_MCRecoParticleLink_true, 'ReconstructedChargedWithPFRICHTOFDIRCLOWQ2PIDParticleLinks',
                ['ReconstructedChargedWithPFRICHTOFDIRCPIDParticleLinks', 'TaggerTrackerReconstructedParticleLinks'],
                ['ReconstructedChargedWithPFRICHTOFDIRCLOWQ2PIDParticleLinks'])
        app.add(meta.CollectionCollector_ReconstructedParticle_true, 'ReconstructedTruthSeededChargedWithPFRICHTOFDIRCLOWQ2PIDParticles',
                ['ReconstructedTruthSeededChargedWithPFRICHTOFDIRCPIDParticles', 'TaggerTrackerReconstructedParticles'],
                ['ReconstructedTruthSeededChargedWithPFRICHTOFDIRCLOWQ2PIDParticles'])
        app.add(meta.CollectionCollector_MCRecoParticleAssociation_true, 'ReconstructedTruthSeededChargedWithPFRICHTOFDIRCLOWQ2PIDParticleAssociations',
                ['ReconstructedTruthSeededChargedWithPFRICHTOFDIRCPIDParticleAssociations', 'TaggerTrackerReconstructedParticleAssociations'],
                ['ReconstructedTruthSeededChargedWithPFRICHTOFDIRCLOWQ2PIDParticleAssociations'])
        app.add(meta.CollectionCollector_MCRecoParticleLink_true, 'ReconstructedTruthSeededChargedWithPFRICHTOFDIRCLOWQ2PIDParticleLinks',
                ['ReconstructedTruthSeededChargedWithPFRICHTOFDIRCPIDParticleLinks', 'TaggerTrackerReconstructedParticleLinks'],
                ['ReconstructedTruthSeededChargedWithPFRICHTOFDIRCLOWQ2PIDParticleLinks'])
        app.add(pid_lut.PIDLookup, 'DRICHTruthSeededLUTPID',
                ['EventHeader', 'ReconstructedTruthSeededChargedWithPFRICHTOFDIRCLOWQ2PIDParticles', 'ReconstructedTruthSeededChargedWithPFRICHTOFDIRCLOWQ2PIDParticleLinks'],
                ['ReconstructedTruthSeededChargedParticles', 'ReconstructedTruthSeededChargedParticleLinks', 'ReconstructedTruthSeededChargedParticleAssociations', 'DRICHTruthSeededParticleIDs'],
                filename='calibrations/drich.lut.gz',
                system='ForwardRICH_ID',
                pdg_values=[211, 321, 2212],
                charge_values=[1],
                momentum_edges=[0.75, 1.25, 1.75, 2.25, 2.75, 3.25, 3.75, 4.25, 4.75, 5.25, 5.75, 6.25, 6.75, 7.25, 7.75, 8.25, 8.75, 9.25, 9.75, 10.25, 10.75, 11.25, 11.75, 12.25, 12.75, 13.25, 13.75, 14.25, 14.75, 15.25, 15.75, 16.25, 16.75, 17.25, 17.75, 18.25, 18.75, 19.25, 19.75, 20.25, 20.75, 21.25, 21.75, 22.25, 22.75, 23.25, 23.75, 24.25, 24.75, 25.25, 25.75, 26.25, 26.75, 27.25, 27.75, 28.25, 28.75, 29.25, 29.75, 30.25, 30.75, 31.25, 33.0, 35.0, 37.0, 39.0, 41.0, 43.0, 45.0, 47.0, 49.0, 51.0, 59.0, 61.0],
                polar_edges=[0.0635, 0.0702, 0.0776, 0.0858, 0.0948, 0.1047, 0.1157, 0.1278, 0.1412, 0.156, 0.1724, 0.1904, 0.2103, 0.2322, 0.2564, 0.283, 0.3123, 0.3445, 0.3799, 0.4188],
                azimuthal_binning=[0.0, 6.283185307179586, 6.283185307179586],
                momentum_bin_centers_in_lut=True,
                polar_bin_centers_in_lut=True,
                use_radians=True,
                missing_electron_prob=True)
        app.add(pid_lut.PIDLookup, 'DRICHLUTPID',
                ['EventHeader', 'ReconstructedWithPFRICHTOFDIRCLOWQ2PIDChargedParticles', 'ReconstructedChargedWithPFRICHTOFDIRCLOWQ2PIDParticleLinks'],
                ['ReconstructedChargedParticles', 'ReconstructedChargedParticleLinks', 'ReconstructedChargedParticleAssociations', 'DRICHParticleIDs'],
                filename='calibrations/drich.lut.gz',
                system='ForwardRICH_ID',
                pdg_values=[211, 321, 2212],
                charge_values=[1],
                momentum_edges=[0.75, 1.25, 1.75, 2.25, 2.75, 3.25, 3.75, 4.25, 4.75, 5.25, 5.75, 6.25, 6.75, 7.25, 7.75, 8.25, 8.75, 9.25, 9.75, 10.25, 10.75, 11.25, 11.75, 12.25, 12.75, 13.25, 13.75, 14.25, 14.75, 15.25, 15.75, 16.25, 16.75, 17.25, 17.75, 18.25, 18.75, 19.25, 19.75, 20.25, 20.75, 21.25, 21.75, 22.25, 22.75, 23.25, 23.75, 24.25, 24.75, 25.25, 25.75, 26.25, 26.75, 27.25, 27.75, 28.25, 28.75, 29.25, 29.75, 30.25, 30.75, 31.25, 33.0, 35.0, 37.0, 39.0, 41.0, 43.0, 45.0, 47.0, 49.0, 51.0, 59.0, 61.0],
                polar_edges=[0.0635, 0.0702, 0.0776, 0.0858, 0.0948, 0.1047, 0.1157, 0.1278, 0.1412, 0.156, 0.1724, 0.1904, 0.2103, 0.2322, 0.2564, 0.283, 0.3123, 0.3445, 0.3799, 0.4188],
                azimuthal_binning=[0.0, 6.283185307179586, 6.283185307179586],
                momentum_bin_centers_in_lut=True,
                polar_bin_centers_in_lut=True,
                use_radians=True,
                missing_electron_prob=True)
