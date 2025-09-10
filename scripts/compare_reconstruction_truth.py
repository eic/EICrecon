#!/usr/bin/env python3
"""
Reconstruction Quality Analysis Script

This script compares reconstructed particle properties against MC truth
for collections where truth-reconstruction associations exist.

It prints quantitative comparisons in terms of reconstructed/truth ratios
to assess how well the reconstruction is performing.
"""

import sys
import argparse
import numpy as np
from pathlib import Path
from collections import defaultdict

try:
    import uproot
    import awkward as ak
except ImportError as e:
    print(f"Error: Required Python packages not available: {e}")
    print("Please install: pip install uproot awkward")
    sys.exit(1)


def read_collections(file_path, collections):
    """Read specified collections from an EDM4hep ROOT file."""
    try:
        with uproot.open(file_path) as file:
            # Get the events tree
            if 'events' not in file:
                print(f"Warning: No 'events' tree found in {file_path}")
                return {}
            
            tree = file['events']
            
            # Check which collections exist
            available_collections = set(tree.keys())
            requested_collections = set(collections)
            
            found_collections = requested_collections.intersection(available_collections)
            missing_collections = requested_collections - available_collections
            
            if missing_collections:
                print(f"Warning: Collections not found in {file_path}: {missing_collections}")
            
            if not found_collections:
                print(f"Warning: No requested collections found in {file_path}")
                return {}
            
            # Read the collections
            print(f"Reading collections from {file_path}: {found_collections}")
            data = tree.arrays(list(found_collections), library="ak")
            
            return data
            
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return {}


def calculate_particle_ratios(mc_particles, reco_particles, associations):
    """Calculate momentum and energy ratios between reconstructed and MC truth particles."""
    
    ratios = {
        'momentum': [],
        'energy': [], 
        'momentum_x': [],
        'momentum_y': [],
        'momentum_z': [],
        'n_matched': 0,
        'n_mc_total': 0,
        'n_reco_total': 0
    }
    
    try:
        # Count total particles
        ratios['n_mc_total'] = ak.sum(ak.num(mc_particles, axis=1))
        ratios['n_reco_total'] = ak.sum(ak.num(reco_particles, axis=1))
        
        if associations is None:
            print("No associations available for particle matching")
            return ratios
            
        # Loop through events
        for event_idx in range(len(associations)):
            if event_idx >= len(mc_particles) or event_idx >= len(reco_particles):
                continue
                
            assoc_event = associations[event_idx]
            mc_event = mc_particles[event_idx]
            reco_event = reco_particles[event_idx]
            
            # Process associations in this event
            for assoc in assoc_event:
                try:
                    # Get MC and reconstructed particle indices from association
                    mc_idx = assoc.sim
                    reco_idx = assoc.rec
                    
                    if mc_idx < len(mc_event) and reco_idx < len(reco_event):
                        mc_p = mc_event[mc_idx]
                        reco_p = reco_event[reco_idx]
                        
                        # Calculate momenta
                        mc_momentum = np.sqrt(mc_p.momentum.x**2 + mc_p.momentum.y**2 + mc_p.momentum.z**2)
                        reco_momentum = np.sqrt(reco_p.momentum.x**2 + reco_p.momentum.y**2 + reco_p.momentum.z**2)
                        
                        if mc_momentum > 0 and reco_momentum > 0:
                            ratios['momentum'].append(reco_momentum / mc_momentum)
                            ratios['n_matched'] += 1
                            
                        if mc_p.momentum.x != 0:
                            ratios['momentum_x'].append(reco_p.momentum.x / mc_p.momentum.x)
                        if mc_p.momentum.y != 0:
                            ratios['momentum_y'].append(reco_p.momentum.y / mc_p.momentum.y) 
                        if mc_p.momentum.z != 0:
                            ratios['momentum_z'].append(reco_p.momentum.z / mc_p.momentum.z)
                            
                        # Calculate energy ratio
                        if mc_p.energy > 0 and reco_p.energy > 0:
                            ratios['energy'].append(reco_p.energy / mc_p.energy)
                            
                except Exception as e:
                    print(f"Error processing association: {e}")
                    continue
                    
    except Exception as e:
        print(f"Error calculating particle ratios: {e}")
        
    return ratios


def calculate_track_ratios(mc_particles, tracks, track_associations):
    """Calculate track parameter ratios between reconstructed and MC truth."""
    
    ratios = {
        'track_momentum': [],
        'n_matched_tracks': 0,
        'n_total_tracks': 0
    }
    
    try:
        if track_associations is None or tracks is None:
            return ratios
            
        ratios['n_total_tracks'] = ak.sum(ak.num(tracks, axis=1))
        
        # Process track associations
        for event_idx in range(len(track_associations)):
            if event_idx >= len(mc_particles) or event_idx >= len(tracks):
                continue
                
            assoc_event = track_associations[event_idx]
            mc_event = mc_particles[event_idx] 
            track_event = tracks[event_idx]
            
            for assoc in assoc_event:
                try:
                    mc_idx = assoc.sim
                    track_idx = assoc.rec
                    
                    if mc_idx < len(mc_event) and track_idx < len(track_event):
                        mc_p = mc_event[mc_idx]
                        track = track_event[track_idx]
                        
                        # Calculate momentum from track parameters if available
                        # This depends on the track parameter format
                        mc_momentum = np.sqrt(mc_p.momentum.x**2 + mc_p.momentum.y**2 + mc_p.momentum.z**2)
                        
                        if hasattr(track, 'momentum') and mc_momentum > 0:
                            track_momentum = np.sqrt(track.momentum.x**2 + track.momentum.y**2 + track.momentum.z**2)
                            if track_momentum > 0:
                                ratios['track_momentum'].append(track_momentum / mc_momentum)
                                ratios['n_matched_tracks'] += 1
                                
                except Exception as e:
                    continue
                    
    except Exception as e:
        print(f"Error calculating track ratios: {e}")
        
    return ratios


def print_analysis_results(ratios, collection_name):
    """Print statistical analysis of the ratios."""
    
    print(f"\n=== {collection_name} Analysis ===")
    
    for ratio_type, values in ratios.items():
        if isinstance(values, list) and len(values) > 0:
            values = np.array(values)
            
            # Filter out extreme outliers (ratios beyond 0.1 to 10)
            filtered_values = values[(values > 0.1) & (values < 10.0)]
            
            if len(filtered_values) > 0:
                mean_ratio = np.mean(filtered_values)
                std_ratio = np.std(filtered_values)
                median_ratio = np.median(filtered_values)
                
                print(f"{ratio_type}:")
                print(f"  Mean ratio: {mean_ratio:.3f} ± {std_ratio:.3f}")
                print(f"  Median ratio: {median_ratio:.3f}")
                print(f"  RMS: {np.sqrt(np.mean(filtered_values**2)):.3f}")
                print(f"  Resolution (σ/μ): {std_ratio/mean_ratio:.3f}")
                print(f"  N matched: {len(filtered_values)} / {len(values)} total")
                
                # Print percentile information
                p25, p75 = np.percentile(filtered_values, [25, 75])
                print(f"  25th-75th percentile: [{p25:.3f}, {p75:.3f}]")
                
        elif isinstance(values, (int, float)):
            print(f"{ratio_type}: {values}")


def analyze_file(file_path, file_type):
    """Analyze a single reconstruction file."""
    
    print(f"\n{'='*60}")
    print(f"Analyzing {file_type}: {Path(file_path).name}")
    print(f"{'='*60}")
    
    # Define collections to read
    collections_to_read = [
        "MCParticles",
        "ReconstructedParticles", 
        "ReconstructedParticleAssociations",
        "CentralCKFTracks",
        "CentralCKFTrackAssociations",
        "CentralCKFTruthSeededTracks", 
        "CentralCKFTruthSeededTrackAssociations"
    ]
    
    # Read data
    data = read_collections(file_path, collections_to_read)
    
    if not data:
        print("No data could be read from file")
        return
    
    # Analyze particles if available
    if all(col in data for col in ["MCParticles", "ReconstructedParticles"]):
        assoc = data.get("ReconstructedParticleAssociations")
        particle_ratios = calculate_particle_ratios(
            data["MCParticles"], 
            data["ReconstructedParticles"], 
            assoc
        )
        print_analysis_results(particle_ratios, "Reconstructed Particles")
        
    # Analyze regular tracks if available
    if all(col in data for col in ["MCParticles", "CentralCKFTracks"]):
        track_assoc = data.get("CentralCKFTrackAssociations")
        track_ratios = calculate_track_ratios(
            data["MCParticles"],
            data["CentralCKFTracks"],
            track_assoc
        )
        print_analysis_results(track_ratios, "Central CKF Tracks")
        
    # Analyze truth-seeded tracks if available
    if all(col in data for col in ["MCParticles", "CentralCKFTruthSeededTracks"]):
        truth_track_assoc = data.get("CentralCKFTruthSeededTrackAssociations")
        truth_track_ratios = calculate_track_ratios(
            data["MCParticles"],
            data["CentralCKFTruthSeededTracks"], 
            truth_track_assoc
        )
        print_analysis_results(truth_track_ratios, "Central CKF Truth-Seeded Tracks")


def main():
    parser = argparse.ArgumentParser(description="Compare reconstruction quality against MC truth")
    parser.add_argument("files", nargs="+", help="EDM4hep ROOT files to analyze")
    parser.add_argument("--output", help="Output file for results (default: stdout)")
    
    args = parser.parse_args()
    
    # Redirect output if requested
    if args.output:
        sys.stdout = open(args.output, 'w')
    
    print("Reconstruction Quality Analysis")
    print("=" * 40)
    print(f"Analyzing {len(args.files)} file(s)")
    
    try:
        for file_path in args.files:
            if not Path(file_path).exists():
                print(f"Warning: File not found: {file_path}")
                continue
                
            # Determine file type from name
            file_name = Path(file_path).name
            if "gun" in file_name:
                file_type = "Particle Gun"
            elif "dis" in file_name:
                file_type = "Deep Inelastic Scattering" 
            else:
                file_type = "Unknown"
                
            analyze_file(file_path, file_type)
            
    except KeyboardInterrupt:
        print("\nAnalysis interrupted by user")
    except Exception as e:
        print(f"Error during analysis: {e}")
        return 1
        
    print(f"\n{'='*60}")
    print("Analysis completed")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())