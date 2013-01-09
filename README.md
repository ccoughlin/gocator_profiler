# Gocator 20x0 Profiler

This is a basic console app to control [Gocator 20x0](http://www.lmi3d.com/product/gocator-family) 3D laser profilers.  I'm mainly using this repo to organize my code as I work through a final application, and it's probably of limited interest to anyone else.

Currently the app works with a Gocator 2020 connected to an RLS LM10IC050 encoder, and outputs range data as a comma-delimited x,y,z file.  You'll need the [Gocator SDK](http://www.lmi3d.com/product/gocator-2000-family/support/files/all).  This repo is Linux-only but it's fairly easy to port to the Windows equivalent; I've only tried with Visual Studio 2008 myself.

Also included is a demo wxPython app to plot the file as a 3D point cloud.

## Requirements

### Profiler
* [Gocator SDK](http://www.lmi3d.com/product/gocator-2000-family/support/files/all)
* [Boost 1.48+](http://www.boost.org)

### Plotter
* [Python](http://www.python.org)
* [NumPy](http://www.numpy.org)
* [matplotlib](http://www.matplotlib.org)
* [wxPython](http://www.wxpython.org)