'''
    A script to visualize the cluster layer-by-layer
    It reads the output from the Juggler component ImagingClusterReco, which is supposed to be clusters of hits after
    digitization, reconstruction, and clustering

    Author: Chao Peng (ANL)
    Date: 04/30/2021
'''

import os
import numpy as np
import pandas as pd
import argparse
import matplotlib
from matplotlib import cm
from matplotlib import pyplot as plt
from matplotlib.ticker import MultipleLocator
from matplotlib.collections import PatchCollection
from matplotlib.patches import Rectangle
from mpl_toolkits.axes_grid1 import make_axes_locatable
from matplotlib.backends.backend_pdf import PdfPages
from utils import *
import sys
import imageio


# color map
def draw_heatmap(axis, x, y, weights, bins=1000, vmin=None, vmax=None, cmap=plt.get_cmap('rainbow'), pc_kw=dict()):
    w, xedg, yedg = np.histogram2d(x, y, weights=weights, bins=bins)
    xsz = np.mean(np.diff(xedg))
    ysz = np.mean(np.diff(yedg))
    if vmin == None:
        vmin = w.min()
    if vmax == None:
        vmax = w.max()
    recs, clrs = [], []
    for i in np.arange(len(xedg) - 1):
        for j in np.arange(len(yedg) - 1):
            if w[i][j] > vmin:
                recs.append(Rectangle((xedg[i], yedg[j]), xsz, ysz))
                clrs.append(cmap((w[i][j] - vmin) / (vmax - vmin)))
    axis.add_collection(PatchCollection(recs, facecolor=clrs, **pc_kw))
    axis.set_xlim(xedg[0], xedg[-1])
    axis.set_ylim(yedg[0], yedg[-1])
    return axis, cm.ScalarMappable(norm=matplotlib.colors.Normalize(vmin=vmin, vmax=vmax), cmap=cmap)


# execute this script
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Visualize the cluster (layer-wise) from analysis')
    parser.add_argument('file', type=str, help='path to root file')
    parser.add_argument('-e', type=int, default=0, dest='iev', help='event number to plot')
    parser.add_argument('-c', type=int, default=0, dest='icl', help='cluster number to plot (0: all, -1: no cluster)')
    parser.add_argument('-s', type=int, default=8, dest='stop', help='stop layer for track fit')
    parser.add_argument('-o', type=str, default='./plots', dest='outdir', help='output directory')
    parser.add_argument('-d', type=float, default=1.0, dest='dura', help='duration of gif')
    parser.add_argument('--compact', type=str, default='', dest='compact', help='compact file')
    parser.add_argument('-m', '--macros', type=str, default='rootlogon.C', dest='macros',
                        help='root macros to load (accept multiple paths separated by \",\")')
    parser.add_argument('-b', '--branch-name', type=str, default='RecoEcalBarrelImagingHits', dest='branch',
                        help='branch name in the root file (outputLayerCollection from ImagingClusterReco)')
    parser.add_argument('--topo-size', type=float, default=2.0, dest='topo_size',
                        help='bin size for projection plot (mrad)')
    parser.add_argument('--topo-range', type=float, default=50.0, dest='topo_range',
                        help='half range for projection plot (mrad)')
    args = parser.parse_args()


    # we can read these values from xml file
    desc = compact_constants(args.compact, [
#        'EcalBarrel_rmin',
#        'EcalBarrel_ReadoutLayerThickness',
#        'EcalBarrel_ReadoutLayerNumber',
#        'EcalBarrel_length',
    ])
    if not len(desc):
        # or define Ecal shapes
        rmin, thickness, length = 890, 20*(10. + 1.65), 860*2+500
    else:
        # convert cm to mm
        rmin = desc[0]*10.
        thickness = desc[1]*desc[2]*10.
        length = desc[3]*10.


    # read data
    load_root_macros(args.macros)
    df = get_hits_data(args.file, args.iev, args.branch)
    if args.icl != 0:
        df = df[df['cluster'] == args.icl]
    if not len(df):
        print("Error: do not find any hits for cluster {:d} in event {:d}".format(args.icl, args.iev))
        exit(-1)
    # convert to polar coordinates (mrad), and stack all r values
    df['r'] = np.sqrt(df['x'].values**2 + df['y'].values**2 + df['z'].values**2)
    df['phi'] = np.arctan2(df['y'].values, df['x'].values)*1000.
    df['theta'] = np.arccos(df['z'].values/df['r'].values)*1000.
    df['eta'] = -np.log(np.tan(df['theta'].values/1000./2.))

    # truth
    dfmcp = get_mcp_simple(args.file, args.iev, 'MCParticles').iloc[0]
    pdgbase = ROOT.TDatabasePDG()
    inpart = pdgbase.GetParticle(int(dfmcp['pid']))
    print("Incoming particle = {}, pdgcode = {}, charge = {}, mass = {}"\
          .format(inpart.GetName(), inpart.PdgCode(), inpart.Charge(), inpart.Mass()))
    # neutral particle, no need to consider magnetic field
    if np.isclose(inpart.Charge(), 0., rtol=1e-5):
        vec = dfmcp[['px', 'py', 'pz']].values
    # charge particle, use the cluster center
    else:
        dfc = df[(df['eta'] <= df['eta'].quantile(0.95)) & (df['eta'] >= df['eta'].quantile(0.05)) &
                 (df['phi'] <= df['phi'].quantile(0.95)) & (df['phi'] >= df['phi'].quantile(0.05))]
        vec = np.average(dfc[['x', 'y', 'z']].values, axis=0, weights= dfc['energy'].values if np.sum(dfc['energy'].values) > 0 else None)
    vec = vec/np.linalg.norm(vec)

    # particle line from (0, 0, 0) to the inner Ecal surface
    length = rmin/np.sqrt(vec[0]**2 + vec[1]**2)
    pline = np.transpose(vec*np.mgrid[0:length:2j][:, np.newaxis])
    cmap = truncate_colormap(plt.get_cmap('jet'), 0.1, 0.9)

    # convert truth to mrad
    vecp = np.asarray([np.arccos(vec[2]), np.arctan2(vec[1], vec[0])])*1000.
    phi_rg = np.asarray([vecp[1] - args.topo_range, vecp[1] + args.topo_range])
    th_rg = np.asarray([vecp[0] - args.topo_range, vecp[0] + args.topo_range])
    eta_rg = np.resize(-np.log(np.tan(vecp[0]/1000./2.)), 2) + np.asarray([-args.topo_range, args.topo_range])/1000.

    os.makedirs(args.outdir, exist_ok=True)

    # cluster plot by layers (rebinned grids)
    pdf = PdfPages(os.path.join(args.outdir, 'e{}_c{}_layers.pdf'.format(args.iev, args.icl)))
    bpos, bprops = (0.5, 0.95), dict(boxstyle='round', facecolor='wheat', alpha=0.2)
    frames = []
    for i in np.arange(1, df['layer'].max() + 1, dtype=int):
        data = df[df['layer'] == i]
        fig, axs = plt.subplots(1, 2, figsize=(17, 16), dpi=160, gridspec_kw={'wspace':0., 'width_ratios': [16, 1]})
        ax, sm = draw_heatmap(axs[0], data['eta'].values, data['phi'].values, weights=data['energy'].values,
                              bins=(np.arange(*eta_rg, step=args.topo_size/1000.), np.arange(*phi_rg, step=args.topo_size)),
                              cmap=cmap, vmin=0.0, vmax=1.0, pc_kw=dict(alpha=0.8, edgecolor='k'))
        ax.set_ylabel(r'$\phi$ (mrad)', fontsize=28)
        ax.set_xlabel(r'$\eta$', fontsize=28)
        ax.tick_params(labelsize=24)
        ax.xaxis.set_minor_locator(MultipleLocator(5))
        ax.yaxis.set_minor_locator(MultipleLocator(5))
        ax.grid(linestyle=':', which='both')
        ax.set_axisbelow(True)
        ax.text(*bpos, 'Layer {:d}'.format(i), transform=ax.transAxes,
                fontsize=26, va='top', ha='center', bbox=bprops)
        cb = plt.colorbar(sm, cax=axs[1], shrink=0.85)
        cb.ax.tick_params(labelsize=24)
        cb.ax.get_yaxis().labelpad = 24
        cb.ax.set_ylabel('Energy Deposit (MeV)', rotation=90, fontsize=28)
        pdf.savefig(fig)
        # gif frames
        fig.savefig('ltmp.png')
        plt.close(fig)
        frames.append(imageio.v2.imread('ltmp.png'))
    pdf.close()
    os.remove('ltmp.png')

    # build GIF
    imageio.mimsave(os.path.join(args.outdir, 'e{:d}_c{:d}_layers.gif'.format(args.iev, args.icl)),
                    frames, 'GIF', duration=args.dura)


    # cluster plot by layers (scatters)
    pdf = PdfPages(os.path.join(args.outdir, 'e{}_c{}_layers2.pdf'.format(args.iev, args.icl)))
    bpos, bprops = (0.5, 0.95), dict(boxstyle='round', facecolor='wheat', alpha=0.2)
    frames = []
    for i in np.arange(1, df['layer'].max() + 1, dtype=int):
        data = df[df['layer'] == i]
        fig, axs = plt.subplots(1, 2, figsize=(17, 16), dpi=160, gridspec_kw={'wspace':0., 'width_ratios': [16, 1]})
        ax = axs[0]
        colors = cmap(data['energy']/1.0)
        ax.scatter(data['eta'].values, data['phi'].values, c=colors, marker='s', s=15.0)
        ax.set_xlim(*eta_rg)
        ax.set_ylim(*phi_rg)
        ax.set_ylabel(r'$\phi$ (mrad)', fontsize=28)
        ax.set_xlabel(r'$\eta$', fontsize=28)
        ax.tick_params(labelsize=24)
        ax.xaxis.set_minor_locator(MultipleLocator(5))
        ax.yaxis.set_minor_locator(MultipleLocator(5))
        ax.grid(linestyle=':', which='both')
        ax.set_axisbelow(True)
        ax.text(*bpos, 'Layer {:d}'.format(i), transform=ax.transAxes,
                fontsize=26, va='top', ha='center', bbox=bprops)
        cb = plt.colorbar(sm, cax=axs[1], shrink=0.85)
        cb.ax.tick_params(labelsize=24)
        cb.ax.get_yaxis().labelpad = 24
        cb.ax.set_ylabel('Energy Deposit (MeV)', rotation=90, fontsize=28)
        pdf.savefig(fig)
        # gif frames
        fig.savefig('ltmp.png')
        plt.close(fig)
        frames.append(imageio.v2.imread('ltmp.png'))
    pdf.close()
    os.remove('ltmp.png')

    # build GIF
    imageio.mimsave(os.path.join(args.outdir, 'e{:d}_c{:d}_layers2.gif'.format(args.iev, args.icl)),
                    frames, 'GIF', duration=args.dura)

