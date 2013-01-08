#!/usr/bin/env python

"""gocator_plotter.py - quick demonstration of plotting Gocator results

Chris R. Coughlin (TRI/Austin, Inc.)
"""

__author__ = 'Chris R. Coughlin'

import wx
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.figure import Figure
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg as FigureCanvas
from matplotlib.backends.backend_wxagg import NavigationToolbar2Wx
import numpy as np

widget_margin = 3 # Default margin around widgets
ctrl_pct = 1.0 # Default to 100% resizing factor for controls
lbl_pct = 0.25 # Default to 25% resizing factor for labels
sizer_flags = wx.ALL | wx.EXPAND # Default resizing flags for controls
lblsizer_flags = wx.ALIGN_CENTRE_VERTICAL | wx.ALL # Default resizing flags for labels

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
        wx.Frame.__init__(self, parent=None, title="Gocator Profile Plotter", *args, **kwargs)
        self.init_ui()

    def init_ui(self):
        """Creates the user interface"""
        self.init_menu()
        self.SetSize(wx.Size(640, 480))
        self.main_panel = wx.Panel(self)
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
        self.plotdata_mnui = wx.MenuItem(self.file_mnu, wx.ID_ANY, text="Plot Data...",
                                         help="Reads and plots an XYZ CSV data file")
        self.Bind(wx.EVT_MENU, self.on_plot_data, id=self.plotdata_mnui.GetId())
        self.file_mnu.AppendItem(self.plotdata_mnui)
        self.setzlim_mnui = wx.MenuItem(self.file_mnu, wx.ID_ANY, text="Set Z Axis Limits...",
                                        help="Sets the limits of the Z Axis")
        self.Bind(wx.EVT_MENU, self.on_setzlim, id=self.setzlim_mnui.GetId())
        self.file_mnu.AppendItem(self.setzlim_mnui)
        self.menubar.Append(self.file_mnu, "Operation")
        self.SetMenuBar(self.menubar)

    def on_plot_data(self, evt):
        """Handles request to plot data"""
        allfiles_wildcard = "All Files (*.*)|*.*"
        txt_wildcards = "Text Files|*.txt;*.csv;*.dat;*.tab;*.asc"
        wildcards = "|".join([txt_wildcards, allfiles_wildcard])
        file_dlg = wx.FileDialog(parent=self, message="Please specify a file", wildcard=wildcards, style=wx.FD_OPEN)
        if file_dlg.ShowModal() == wx.ID_OK:
            self.plot_data(file_dlg.GetPath())

    def plot_data(self, data_fname):
        """Reads and plots the specified data file"""
        self.figure.clf()
        self.axes = self.figure.add_subplot(111, projection='3d')
        self.axes.grid(True)
        try:
            wx.BeginBusyCursor()
            x, y, z = np.genfromtxt(data_fname, delimiter=",", unpack=True)
            self.axes.plot(x, y, z, c='r', linestyle='', marker='.')
            self.axes.set_xlabel('X Position [mm]')
            self.axes.set_ylabel('Y Position [mm]')
            self.axes.set_zlabel('Z Position [mm]')
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

    def on_setzlim(self, evt):
        """Handles request to reset the Z axis limits"""
        if hasattr(self, 'axes'):
            rng_dlg = FloatRangeDialog(dlg_title="Set Z Axis Limits")
            try:
                min_val, max_val = self.axes.get_zlim()
            except AttributeError: # different matplotlib version, API change
                min_val, max_val = self.axes.get_zlim3d()
            rng_dlg.setValues(min_val, max_val)
            if rng_dlg.ShowModal() == wx.ID_OK:
                min_val, max_val = rng_dlg.GetValue()
                if min_val is not None and max_val is not None:
                    try:
                        self.axes.set_zlim((min_val, max_val))
                    except AttributeError: #different matplotlib version
                        self.axes.set_zlim3d((min_val, max_val))
                    self.refresh_plot()
            rng_dlg.Destroy()

    def refresh_plot(self):
        """Forces plot to redraw itself"""
        self.canvas.draw()

def main():
    app = wx.PySimpleApp()
    ui = UI()
    ui.Show(True)
    app.MainLoop()

if __name__ == "__main__":
    main()
