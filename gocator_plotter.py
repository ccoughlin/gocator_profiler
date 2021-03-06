#!/usr/bin/env python

"""gocator_plotter.py - quick demonstration of plotting Gocator results

Chris R. Coughlin (TRI/Austin, Inc.)
"""

__author__ = 'Chris R. Coughlin'

import wx
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.mlab import griddata
from matplotlib import cm
from matplotlib.figure import Figure
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg as FigureCanvas
from matplotlib.backends.backend_wxagg import NavigationToolbar2Wx
import numpy as np
import scipy.signal
import os.path
import sys
import fileinput

widget_margin = 3 # Default margin around widgets
ctrl_pct = 1.0 # Default to 100% resizing factor for controls
lbl_pct = 0.25 # Default to 25% resizing factor for labels
sizer_flags = wx.ALL | wx.EXPAND # Default resizing flags for controls
lblsizer_flags = wx.ALIGN_CENTRE_VERTICAL | wx.ALL # Default resizing flags for labels

class FileDropTarget(wx.FileDropTarget):

    def __init__(self, window):
        super(FileDropTarget, self).__init__()
        self.window = window

    def OnDropFiles(self, x, y, filenames):
        self.window.plot_data(filenames[0])

class FloatRangeDialog(wx.Dialog):
    """Dialog to specify a floating-point numeric range.  Defaults to allowing any
    floating-point value (system-dependent) for both start and finish values.  Returns
    a tuple (start, finish) if accepted."""

    def __init__(self, dlg_title, dlg_msg='Please enter a range.',
                 start_range_lbl='Min', finish_range_lbl='Max'):
        super(FloatRangeDialog, self).__init__(parent=None, title=dlg_title)
        vbox = wx.BoxSizer(wx.VERTICAL)
        msg_text = wx.StaticText(self, wx.ID_ANY, dlg_msg)
        vbox.Add(msg_text, 0, sizer_flags, widget_margin)
        hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        self.start_ctrl = wx.TextCtrl(self, wx.ID_ANY, pos=wx.DefaultPosition,
                                  size=wx.DefaultSize)
        hbox1.Add(wx.StaticText(self, -1, start_range_lbl), 1, lblsizer_flags,
                  widget_margin)
        hbox1.Add(self.start_ctrl, 0, sizer_flags, widget_margin)
        hbox2 = wx.BoxSizer(wx.HORIZONTAL)
        self.finish_ctrl = wx.TextCtrl(self, wx.ID_ANY, pos=wx.DefaultPosition,
                                   size=wx.DefaultSize)
        hbox2.Add(wx.StaticText(self, -1, finish_range_lbl), 1, lblsizer_flags,
                  widget_margin)
        hbox2.Add(self.finish_ctrl, 0, sizer_flags, widget_margin)
        hbox3 = wx.BoxSizer(wx.HORIZONTAL)
        btns = self.CreateButtonSizer(wx.OK | wx.CANCEL)
        hbox3.Add(btns, 1, sizer_flags, widget_margin)
        vbox.Add(hbox1, 0, sizer_flags, 0)
        vbox.Add(hbox2, 0, sizer_flags, 0)
        vbox.Add(hbox3, 0, wx.ALIGN_RIGHT, 0)
        self.SetSizer(vbox)
        self.SetInitialSize()

    def GetValue(self):
        """Returns the tuple (start,finish) if the dialog was accepted."""
        try:
            min_val = float(self.start_ctrl.GetValue())
            max_val = float(self.finish_ctrl.GetValue())
            return min_val, max_val
        except ValueError:
            return None,None

    def setValues(self, min_val=None, max_val=None):
        """Sets the minimum and maximum values if specified."""
        if min_val is not None:
            self.start_ctrl.SetValue(str(min_val))
        if max_val is not None:
            self.finish_ctrl.SetValue(str(max_val))

class UI(wx.Frame):
    """Basic wxPython user interface"""

    def __init__(self, *args, **kwargs):
        self.base_title = "Gocator Profile Plotter"
        wx.Frame.__init__(self, parent=None, title=self.base_title, *args, **kwargs)
        self.init_ui()
        self.data_fname = None
        self.point_color = (0.31, 0.40, 0.51)

    def init_ui(self):
        """Creates the user interface"""
        self.init_menu()
        self.SetSize(wx.Size(640, 480))
        self.main_panel = wx.Panel(self)
        file_drop_target = FileDropTarget(self)
        self.main_panel.SetDropTarget(file_drop_target)
        self.main_panel_sizer = wx.BoxSizer(wx.VERTICAL)
        self.figure = Figure()
        self.canvas = FigureCanvas(self.main_panel, wx.ID_ANY, self.figure)
        self.main_panel_sizer.Add(self.canvas, 1, sizer_flags, 0)
        self.toolbar = NavigationToolbar2Wx(self.canvas)
        self.toolbar.Realize()
        if wx.Platform == '__WXMAC__':
            self.SetToolBar(self.toolbar)
        else:
            tw, th = self.toolbar.GetSizeTuple()
            fw, fh = self.canvas.GetSizeTuple()
            self.toolbar.SetSize(wx.Size(fw, th))
            self.main_panel_sizer.Add(self.toolbar, 0, wx.LEFT | wx.EXPAND, 0)
        self.toolbar.update()
        self.main_panel.SetSizerAndFit(self.main_panel_sizer)

    def init_menu(self):
        """Creates the main menu"""
        self.menubar = wx.MenuBar()
        self.file_mnu = wx.Menu()
        self.plotdata_mnui = wx.MenuItem(self.file_mnu, wx.ID_ANY, text="Plot Data...\tCTRL+O",
                                         help="Reads and plots an XYZ CSV data file")
        self.Bind(wx.EVT_MENU, self.on_plot_data, id=self.plotdata_mnui.GetId())
        self.file_mnu.AppendItem(self.plotdata_mnui)
        self.quit_mnui = wx.MenuItem(self.file_mnu, wx.ID_ANY, text="Exit Plotter\tCTRL+Q")
        self.Bind(wx.EVT_MENU, self.on_quit, id=self.quit_mnui.GetId())
        self.file_mnu.AppendItem(self.quit_mnui)
        self.menubar.Append(self.file_mnu, "File")
        
        self.plot_mnu = wx.Menu()
        plottype_mnu = wx.Menu()
        self.plot_pointcloud_mnui = wx.MenuItem(plottype_mnu, wx.ID_ANY, text="Point Cloud",
                                                help="Plots the laser profile as a point cloud", kind=wx.ITEM_RADIO)
        self.Bind(wx.EVT_MENU, self.on_plot_type, id=self.plot_pointcloud_mnui.GetId())
        plottype_mnu.AppendItem(self.plot_pointcloud_mnui)
        self.plot_surface_mnui = wx.MenuItem(plottype_mnu, wx.ID_ANY, text="Fitted Surface",
                                             help="Plots the laser profile as an interpolated surface", kind=wx.ITEM_RADIO)
        self.Bind(wx.EVT_MENU, self.on_plot_type, id=self.plot_surface_mnui.GetId())
        plottype_mnu.AppendItem(self.plot_surface_mnui)
        self.plot_wireframe_mnui = wx.MenuItem(plottype_mnu, wx.ID_ANY, text="Fitted Wireframe Surface",
                                                help="Plots the laser profile as an interpolated wireframe surface", 
                                                kind=wx.ITEM_RADIO)
        plottype_mnu.AppendItem(self.plot_wireframe_mnui)
        self.Bind(wx.EVT_MENU, self.on_plot_type, id=self.plot_wireframe_mnui.GetId())
        self.plot_mnu.AppendSubMenu(plottype_mnu, "Type Of Plot")
        
        self.plot_color_mnui = wx.MenuItem(self.plot_mnu, wx.ID_ANY, text="Choose Point Color", help="Select the point color to use in point cloud plots")
        self.Bind(wx.EVT_MENU, self.on_choosepointcolor, id=self.plot_color_mnui.GetId())
        self.plot_mnu.AppendItem(self.plot_color_mnui)
        
        self.plotrawdata = wx.MenuItem(self.plot_mnu, wx.ID_ANY, text="Plot Raw Data",
                                           help="Plot raw vs. filtered data", kind=wx.ITEM_CHECK)
        self.Bind(wx.EVT_MENU, self.on_filter_data, id=self.plotrawdata.GetId())
        self.plot_mnu.AppendItem(self.plotrawdata)
        self.setzlim_mnui = wx.MenuItem(self.plot_mnu, wx.ID_ANY, text="Set Z Axis Limits...\tCTRL+Z",
                                        help="Sets the limits of the Z Axis")
        self.Bind(wx.EVT_MENU, self.on_setzlim, id=self.setzlim_mnui.GetId())
        self.plot_mnu.AppendItem(self.setzlim_mnui)
        
        self.menubar.Append(self.plot_mnu, "Plot")
        self.SetMenuBar(self.menubar)

    def on_plot_data(self, evt):
        """Handles request to plot data"""
        allfiles_wildcard = "All Files (*.*)|*.*"
        txt_wildcards = "Text Files|*.txt;*.csv;*.dat;*.tab;*.asc"
        wildcards = "|".join([txt_wildcards, allfiles_wildcard])
        file_dlg = wx.FileDialog(parent=self, message="Please specify a file", wildcard=wildcards, style=wx.FD_OPEN)
        if file_dlg.ShowModal() == wx.ID_OK:
            #self.data_fname = file_dlg.GetPath()
            #self.plot_data(self.data_fname)
            #self.SetTitle(' - '.join([self.base_title, os.path.basename(self.data_fname)]))
            self.plot_data(file_dlg.GetPath())

    def plot_data(self, data_fname):
        """Reads and plots the specified data file"""
        self.data_fname = data_fname
        self.SetTitle(' - '.join([self.base_title, os.path.basename(self.data_fname)]))
        self.figure.clf()
        self.axes = self.figure.add_subplot(111, projection='3d')
        self.axes.grid(True)
        try:
            wx.BeginBusyCursor()
            x, y, z = self.get_data(data_fname)
            if self.plot_pointcloud_mnui.IsChecked():
                # Plot a point cloud
                self.axes.plot(x, y, z, c=self.point_color, linestyle='', marker=',', rasterized=True)
            else:
                xi = np.linspace(min(x), max(x), num=100)
                yi = np.linspace(min(y), max(y), num=100)
                X, Y = np.meshgrid(xi, yi)
                Z = griddata(x, y, z, xi, yi)
                if self.plot_surface_mnui.IsChecked():
                    # Interpolate a surface from the point cloud to plot
                    surf = self.axes.plot_surface(X, Y, Z, rstride=3, cstride=3, cmap=cm.get_cmap('spectral'), 
                                              linewidth=1, antialiased=True)
                    colorbar = self.figure.colorbar(surf)
                    colorbar.set_label("Z Position [mm]")
                elif self.plot_wireframe_mnui.IsChecked():
                    # Interpolate a wireframe surface from the point cloud to plot
                    # Uses a higher resolution presentation than the surface - surface tends to bog down
                    # on some systems with stride set to 1
                    surf = self.axes.plot_wireframe(X, Y, Z, rstride=1, cstride=1, color=self.point_color,
                                              linewidth=1, antialiased=True)
                self.axes.set_zlim3d(np.min(Z), np.max(Z))
            self.axes.set_xlabel('X Position [mm]')
            self.axes.set_ylabel('Y Position [mm]')
            self.axes.set_zlabel('Z Position [mm]')
            self.axes.set_title(os.path.basename(self.data_fname))
            
            self.refresh_plot()
        except IOError: # no data
            err_dlg = wx.MessageDialog(self, caption="Invalid Data File",
                                       message="Unable to read data from {0}.".format(data_fname),
                                       style=wx.ICON_ERROR)
            err_dlg.ShowModal()
            err_dlg.Destroy()
            return
        except ValueError: # couldn't convert to floating point
            err_dlg = wx.MessageDialog(self, caption="Invalid Input",
                                       message="An error occurred reading the data.\nPlease ensure the data is properly formatted.",
                                       style=wx.ICON_ERROR)
            err_dlg.ShowModal()
            err_dlg.Destroy()
            return
        finally:
            wx.EndBusyCursor()

    def get_data(self, data_fname):
        """Reads the specified data file, optionally filtering the data before returning it as X,Y,Z."""
        x, y, z = np.genfromtxt(data_fname, delimiter=",", unpack=True)
        if not self.plotrawdata.IsChecked():
            xi = x[z>-20]
            yi = y[z>-20]
            zi = scipy.signal.wiener(z[z>-20], mysize=15, noise=1)
        else:
            xi = x
            yi = y
            zi = z
        return xi, yi, zi
        
    def get_zlim(self):
        """Wrapper for matplotlib API changes in retrieving Z limits"""
        try:
            min_val, max_val = self.axes.get_zlim()
        except AttributeError:
            min_val, max_val = self.axes.get_zlim3d()
        finally:
            return min_val, max_val
            
    def set_zlim(self, limits_tuple):
        """Wrapper for matplotlib API changes in setting Z limits"""
        try:
            self.axes.set_zlim(limits_tuple)
        except AttributeError:
            self.axes.set_zlim3d(limits_tuple)

    def on_setzlim(self, evt):
        """Handles request to reset the Z axis limits"""
        if hasattr(self, 'axes'):
            rng_dlg = FloatRangeDialog(dlg_title="Set Z Axis Limits")
            min_val, max_val = self.get_zlim()
            rng_dlg.setValues(min_val, max_val)
            if rng_dlg.ShowModal() == wx.ID_OK:
                min_val, max_val = rng_dlg.GetValue()
                if min_val is not None and max_val is not None:
                    self.set_zlim((min_val, max_val))
                    self.refresh_plot()
            rng_dlg.Destroy()

    def on_plot_type(self, evt):
        """Handles change in choice of plot type"""
        if hasattr(self, 'data_fname'):
            if self.data_fname is not None:
                axis_limits = self.get_zlim()
                self.plot_data(self.data_fname)
                self.set_zlim(axis_limits)
                self.refresh_plot()
                
    def on_filter_data(self, evt):
        """Handles change in choice of filtering data"""
        if hasattr(self, 'data_fname'):
            if self.data_fname is not None:
                self.plot_data(self.data_fname)

    def refresh_plot(self):
        """Forces plot to redraw itself"""
        self.canvas.draw()
        
    def on_choosepointcolor(self, evt):
        """Handles request to change plot point color"""
        color_dlg = wx.ColourDialog(self)
        color_dlg.GetColourData().SetChooseFull(True)
        if color_dlg.ShowModal() == wx.ID_OK:
            color_data = color_dlg.GetColourData()
            self.point_color = [component/255. for component in color_data.GetColour().Get()]
            if not self.plot_surface_mnui.IsChecked():
                #self.re
                self.plot_data(self.data_fname)

    def on_quit(self, evt):
        """Handles request to exit the program"""
        self.Destroy()

def main(fname=None):
    app = wx.PySimpleApp()
    ui = UI()
    ui.Show(True)
    if fname is not None:
        ui.plot_data(fname)
    app.MainLoop()

if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) > 0:
        fname = args[0]
    else:
        fname = None
    main(fname)
