#!/usr/bin/env python3

import uproot
import numpy as np
import matplotlib.pyplot as plt
import sys

def analyze_particles(tree, detector_name):
    """Analyze particle data for a given detector (DRICH or PFRICH)"""
    
    # Read particle data
    particles_branch = f"{detector_name}IrtParticles"
    
    try:
        pdg = tree[f"{particles_branch}/{particles_branch}.PDG"].array()
        nhits = tree[f"{particles_branch}/{particles_branch}.nhits"].array()
        npe = tree[f"{particles_branch}/{particles_branch}.npe"].array()
        radiators_begin = tree[f"{particles_branch}/{particles_branch}.radiators_begin"].array()
        radiators_end = tree[f"{particles_branch}/{particles_branch}.radiators_end"].array()
        
        # Calculate number of radiators
        n_radiators = radiators_end - radiators_begin
        
        # Flatten arrays (since they're jagged arrays from multiple events)
        pdg_flat = np.concatenate([event for event in pdg if len(event) > 0])
        nhits_flat = np.concatenate([event for event in nhits if len(event) > 0])
        npe_flat = np.concatenate([event for event in npe if len(event) > 0])
        n_radiators_flat = np.concatenate([event for event in n_radiators if len(event) > 0])
        
        print(f"\n{detector_name} Particle Analysis:")
        print(f"  Total particles: {len(pdg_flat)}")
        print(f"  PDG codes: {np.unique(pdg_flat)}")
        print(f"  Hits range: {np.min(nhits_flat)} - {np.max(nhits_flat)}")
        print(f"  NPE range: {np.min(npe_flat):.2f} - {np.max(npe_flat):.2f}")
        print(f"  Radiators range: {np.min(n_radiators_flat)} - {np.max(n_radiators_flat)}")
        
        return pdg_flat, nhits_flat, npe_flat, n_radiators_flat
        
    except Exception as e:
        print(f"Error analyzing {detector_name} particles: {e}")
        return None, None, None, None

def plot_analysis(drich_data, pfrich_data):
    """Create plots for the particle analysis"""
    
    fig, axes = plt.subplots(2, 4, figsize=(16, 10))
    fig.suptitle('RICH Detector Particle Analysis', fontsize=16)
    
    detectors = [('DRICH', drich_data), ('PFRICH', pfrich_data)]
    
    for det_idx, (det_name, data) in enumerate(detectors):
        pdg, nhits, npe, n_radiators = data
        
        if pdg is None:
            continue
            
        # PDG distribution
        axes[det_idx, 0].hist(pdg, bins=50, alpha=0.7, edgecolor='black')
        axes[det_idx, 0].set_title(f'{det_name} PDG Codes')
        axes[det_idx, 0].set_xlabel('PDG Code')
        axes[det_idx, 0].set_ylabel('Count')
        
        # Number of hits
        axes[det_idx, 1].hist(nhits, bins=50, alpha=0.7, edgecolor='black', color='orange')
        axes[det_idx, 1].set_title(f'{det_name} Number of Hits')
        axes[det_idx, 1].set_xlabel('Number of Hits')
        axes[det_idx, 1].set_ylabel('Count')
        
        # Number of photoelectrons
        axes[det_idx, 2].hist(npe, bins=50, alpha=0.7, edgecolor='black', color='green')
        axes[det_idx, 2].set_title(f'{det_name} Number of Photoelectrons')
        axes[det_idx, 2].set_xlabel('NPE')
        axes[det_idx, 2].set_ylabel('Count')
        
        # Number of radiators
        axes[det_idx, 3].hist(n_radiators, bins=max(1, int(np.max(n_radiators)) + 1), 
                             alpha=0.7, edgecolor='black', color='red')
        axes[det_idx, 3].set_title(f'{det_name} Number of Radiators')
        axes[det_idx, 3].set_xlabel('Number of Radiators')
        axes[det_idx, 3].set_ylabel('Count')
    
    plt.tight_layout()
    plt.savefig('rich_analysis.png', dpi=300, bbox_inches='tight')
    plt.show()

def main():
    # Open the ROOT file
    filename = "reco.edm4eic.root"
    
    try:
        with uproot.open(filename) as file:
            print(f"Opened file: {filename}")
            
            # Get the events tree
            tree = file["events"]
            print(f"Number of events: {tree.num_entries}")
            
            # Analyze both detectors
            drich_data = analyze_particles(tree, "DRICH")
            pfrich_data = analyze_particles(tree, "PFRICH")
            
            # Create plots
            plot_analysis(drich_data, pfrich_data)
            
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
        sys.exit(1)
    except Exception as e:
        print(f"Error reading file: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
