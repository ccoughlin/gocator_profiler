# -*- coding: utf-8 -*-
"""
Created on Wed Feb  6 08:00:57 2013

@author: ccoughlin
"""

import glob
import multiprocessing
import os
import os.path
import sys

import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.cm as cm
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure

def generate_scatterplot(datafname, imgfname):
    """Creates a 2D scatter plot of the specified Gocator XYZ
    data and saves it in the specified image filename."""
    matplotlib.rcParams['axes.formatter.limits'] = -4, 4
    matplotlib.rcParams['font.size'] = 14
    matplotlib.rcParams['axes.titlesize'] = 12
    matplotlib.rcParams['axes.labelsize'] = 12
    matplotlib.rcParams['xtick.labelsize'] = 11
    matplotlib.rcParams['ytick.labelsize'] = 11
    figure = Figure()
    canvas = FigureCanvas(figure)
    axes = figure.gca()
    x,y,z = np.genfromtxt(datafname, delimiter=",", unpack=True)
    xi = x[z!=-32.768]
    yi = y[z!=-32.768]
    zi = z[z!=-32.768]
    scatter_plt = axes.scatter(xi, yi, c=zi, cmap=cm.get_cmap("Set1"))
    axes.grid(True)
    axes.axis([np.min(xi), np.max(xi), np.min(yi), np.max(yi)])
    axes.set_title(os.path.basename(datafname))
    colorbar = figure.colorbar(scatter_plt)
    colorbar.set_label("Range [mm]")
    axes.set_xlabel("Horizontal Position [mm]")
    axes.set_ylabel("Scan Position [mm]")
    figure.savefig(imgfname)

def generate_plotfname(datafname, extension='png'):
    """Returns an image filename based on the supplied data filename.
    Defaults to .PNG unless specified otherwise."""
    basename = os.path.basename(datafname)
    folder = os.path.dirname(datafname)
    base, ext = os.path.splitext(basename)
    new_basename = '.'.join([base, extension])
    return os.path.join(folder, new_basename)
    
if __name__ == "__main__":
    multiprocessing.freeze_support()
    try:
        file_list = glob.glob(sys.argv[1])
    except IndexError: # no args specified
        print("Usage: batch_plotter.py files_to_plot")
        print("e.g. batch_plotter.py data/*.csv")
        sys.exit(0)
    if multiprocessing.cpu_count() == 1:
        print("Single CPU detected, running one process.")
        for fname in file_list:
            generate_scatterplot(fname, generate_plotfname(fname))
    else:
        num_processes = multiprocessing.cpu_count() - 1
        pool = multiprocessing.Pool(processes=num_processes)
        print("Multiple CPUs detected, running {0} processes.".format(num_processes))
        for fname in file_list:
            pool.apply_async(generate_scatterplot,
                             args=(fname, generate_plotfname(fname)))
        pool.close()
        pool.join()
    print("\nPlotting complete.")

    