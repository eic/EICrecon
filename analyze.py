#!/usr/bin/env python3

import uproot
import sys

def main():
    # Open the ROOT file
    filename = "reco.edm4eic.root"
    
    try:
        with uproot.open(filename) as file:
            print(f"Opened file: {filename}")
            print("\nAvailable trees:")
            for key in file.keys():
                print(f"  {key}")
            
            # Get the main tree (usually called "events")
            if "events" in file:
                tree = file["events"]
                print(f"\nBranches in 'events' tree ({len(tree.keys())} total):")
                for branch in sorted(tree.keys()):
                    print(f"  {branch}")
            else:
                # If no "events" tree, show branches from first available tree
                tree_name = list(file.keys())[0]
                tree = file[tree_name]
                print(f"\nBranches in '{tree_name}' tree ({len(tree.keys())} total):")
                for branch in sorted(tree.keys()):
                    print(f"  {branch}")
                    
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
        sys.exit(1)
    except Exception as e:
        print(f"Error reading file: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
